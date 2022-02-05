/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
//
// flashlight.cpp
//
// implementation of CHudFlashlight class
//

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

#include "event_api.h"

#include <float.h>
#include <string.h>
#include <stdio.h>



DECLARE_MESSAGE(m_Flash, FlashBat)
DECLARE_MESSAGE(m_Flash, Flashlight)

#define BAT_NAME "sprites/%d_Flashlight.spr"

bool CHudFlashlight::Init()
{
	m_fFade = 0;
	m_fOn = false;
	m_pLight = nullptr;

	HOOK_MESSAGE(Flashlight);
	HOOK_MESSAGE(FlashBat);

	m_iFlags |= HUD_ACTIVE;

	gHUD.AddHudElem(this);

	return true;
}

void CHudFlashlight::Reset()
{
	m_fFade = 0;
	m_fOn = false;
	m_pLight = nullptr;
}

bool CHudFlashlight::VidInit()
{
	int HUD_flash_empty = gHUD.GetSpriteIndex("flash_empty");
	int HUD_flash_full = gHUD.GetSpriteIndex("flash_full");
	int HUD_flash_beam = gHUD.GetSpriteIndex("flash_beam");

	m_hSprite1 = gHUD.GetSprite(HUD_flash_empty);
	m_hSprite2 = gHUD.GetSprite(HUD_flash_full);
	m_hBeam = gHUD.GetSprite(HUD_flash_beam);
	m_prc1 = &gHUD.GetSpriteRect(HUD_flash_empty);
	m_prc2 = &gHUD.GetSpriteRect(HUD_flash_full);
	m_prcBeam = &gHUD.GetSpriteRect(HUD_flash_beam);
	m_iWidth = m_prc2->right - m_prc2->left;

	const int iFlashOverlaySpriteIndex = gHUD.GetSpriteIndex( "flash_overlay" );
	m_hOverlay = gHUD.GetSprite( iFlashOverlaySpriteIndex );
	const Rect *hOverlayRect = &gHUD.GetSpriteRect( iFlashOverlaySpriteIndex );
	m_iOverlaySize = hOverlayRect->right - hOverlayRect->left;

	return true;
}

bool CHudFlashlight::MsgFunc_FlashBat(const char* pszName, int iSize, void* pbuf)
{


	BEGIN_READ(pbuf, iSize);
	int x = READ_BYTE();
	m_iBat = x;
	m_flBat = ((float)x) / 100.0;

	return true;
}

bool CHudFlashlight::MsgFunc_Flashlight(const char* pszName, int iSize, void* pbuf)
{

	BEGIN_READ(pbuf, iSize);
	m_fOn = READ_BYTE() != 0;
	int x = READ_BYTE();
	m_iBat = x;
	m_flBat = ((float)x) / 100.0;

	return true;
}

bool CHudFlashlight::Draw(float flTime)
{
	if ((gHUD.m_iHideHUDDisplay & (HIDEHUD_FLASHLIGHT | HIDEHUD_ALL)) != 0)
		return true;

	int r, g, b, x, y, a;
	Rect rc;

	if (!gHUD.HasSuit())
		return true;

	if (m_fOn)
		a = 225;
	else
		a = MIN_ALPHA;

	if (m_flBat < 0.20)
		UnpackRGB(r, g, b, RGB_REDISH);
	else
		UnpackRGB(r, g, b, RGB_YELLOWISH);

	ScaleColors(r, g, b, a);

	y = (m_prc1->bottom - m_prc2->top) / 2;
	x = ScreenWidth - m_iWidth - m_iWidth / 2;

	// Draw the flashlight casing
	SPR_Set(m_hSprite1, r, g, b);
	SPR_DrawAdditive(0, x, y, m_prc1);

	if (m_fOn)
	{ // draw the flashlight beam
		x = ScreenWidth - m_iWidth / 2;

		SPR_Set(m_hBeam, r, g, b);
		SPR_DrawAdditive(0, x, y, m_prcBeam);
	}

	// draw the flashlight energy level
	x = ScreenWidth - m_iWidth - m_iWidth / 2;
	int iOffset = m_iWidth * (1.0 - m_flBat);
	if (iOffset < m_iWidth)
	{
		rc = *m_prc2;
		rc.left += iOffset;

		SPR_Set(m_hSprite2, r, g, b);
		SPR_DrawAdditive(0, x + iOffset, y, &rc);
	}

	UpdateFlashlight();

	return true;
}

float CHudFlashlight::GetFlashlightRadius( const Vector vecSrc, const pmtrace_t tr )
{
#ifdef DEBUG
	const float flDecayRange = gHUD.m_pFlashlightCvarDecayRange->value;
	const float flRadius = gHUD.m_pFlashlightCvarRadius->value;
#else
	const float flDecayRange = 512.0f;
	const float flRadius = 180.0f;
#endif

	const float flDist = fabs( (tr.endpos - vecSrc).Length() );
	if ( flDist > flDecayRange )
	{
		m_flSpotScale = 0.0f;
		return flRadius;
	}

	m_flSpotScale = flDist / flDecayRange;
	return fmax( 0.01f, flRadius * m_flSpotScale ); // Don't use 0, otherwise the light is killed!
}

void CHudFlashlight::UpdateFlashlight()
{
	if ( !m_fOn )
	{
		if ( m_pLight )
		{
			memset( m_pLight, 0, sizeof( *m_pLight ) );
			m_pLight = nullptr;
		}

		return;
	}

	cl_entity_t *pPlayer = gEngfuncs.GetLocalPlayer();
	if ( !pPlayer )
		return;

	Vector vecAngles;
	gEngfuncs.GetViewAngles( vecAngles );

	Vector vecForward;
	AngleVectors( vecAngles, vecForward, nullptr, nullptr );

	Vector vecEnd;
#ifdef DEBUG
	VectorMA( pPlayer->origin, gHUD.m_pFlashlightCvarRange->value, vecForward, vecEnd );
#else
	VectorMA( pPlayer->origin, 2048.0f, vecForward, vecEnd );
#endif

	pmtrace_t tr;
	gEngfuncs.pEventAPI->EV_SetSolidPlayers( pPlayer->index - 1 );
	gEngfuncs.pEventAPI->EV_SetTraceHull( 2 );
	gEngfuncs.pEventAPI->EV_PlayerTrace( pPlayer->origin, vecEnd, PM_STUDIO_BOX, -1, &tr );

	if ( !m_pLight )
	{
		m_pLight = gEngfuncs.pEfxAPI->CL_AllocDlight( 1 );
		m_pLight->die = FLT_MAX;
	}

	m_vecSpotOrigin = tr.endpos;
	m_pLight->origin = tr.endpos;
	m_pLight->radius = GetFlashlightRadius( pPlayer->origin, tr );
#ifdef DEBUG
	m_pLight->color.r = gHUD.m_pFlashlightCvarRed->value;
	m_pLight->color.g = gHUD.m_pFlashlightCvarGreen->value;
	m_pLight->color.b = gHUD.m_pFlashlightCvarBlue->value;
#else
	m_pLight->color.r = 255.0f;
	m_pLight->color.g = 255.0f;
	m_pLight->color.b = 255.0f;
#endif
}
