//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================

// Triangle rendering, if any

#include "hud.h"
#include "cl_util.h"

// Triangle rendering apis are in gEngfuncs.pTriAPI

#include "const.h"
#include "entity_state.h"
#include "cl_entity.h"
#include "triangleapi.h"
#include "Exports.h"

/*
=================
HUD_DrawNormalTriangles

Non-transparent triangles-- add them here
=================
*/
void DLLEXPORT HUD_DrawNormalTriangles()
{
	//	RecClDrawNormalTriangles();

	gHUD.m_Spectator.DrawOverview();
}


/*
=================
HUD_DrawTransparentTriangles

Render any triangles with transparent rendermode needs here
=================
*/
void DLLEXPORT HUD_DrawTransparentTriangles()
{
	//	RecClDrawTransparentTriangles();

	// TODO: This will likely NOT be multiplayer friendly
	cl_entity_t *pPlayer = gEngfuncs.GetLocalPlayer();
	if ( !pPlayer )
		return;

	if ( !gHUD.m_Flash.m_fOn )
		return;

	struct model_s *hSprite = (struct model_s *)gEngfuncs.GetSpritePointer( gHUD.m_Flash.m_hOverlay );
	if ( !hSprite )
		return;

	if ( gEngfuncs.pTriAPI->SpriteTexture( hSprite, 0 ) == 0 )
		return;

	Vector vecAngles;
	gEngfuncs.GetViewAngles( vecAngles );

	Vector vecForward;
	Vector vecRight;
	Vector vecUp;
	AngleVectors( vecAngles, vecForward, vecRight, vecUp );

	// TODO - Query the sprite size at runtime?
	const float flSpriteSize = 256.0f;
	// TODO - Find a better way the position of the sprite
	// On flat walls, it's fine enough, but on pillars or corners, this is going to look ugly
	Vector vecOrigin = gHUD.m_Flash.m_vecSpotOrigin;
	vecOrigin = vecOrigin + vecRight * -flSpriteSize / 2.0f;
	vecOrigin = vecOrigin + vecUp * -flSpriteSize / 2.0f;
	vecOrigin = vecOrigin + vecForward * -16.0f;

	gEngfuncs.pTriAPI->RenderMode( kRenderTransAdd );
	gEngfuncs.pTriAPI->CullFace( TRI_NONE );
	gEngfuncs.pTriAPI->Begin( TRI_QUADS );

	gEngfuncs.pTriAPI->Color4f( gHUD.m_pFlashlightCvarRed->value / 255.0f, gHUD.m_pFlashlightCvarGreen->value / 255.0f, gHUD.m_pFlashlightCvarBlue->value / 255.0f, 0.5f );
	gEngfuncs.pTriAPI->Brightness( 0.5f );

	gEngfuncs.pTriAPI->TexCoord2f( 1.0f, 1.0f );
	Vector vecPoint = vecOrigin;
	gEngfuncs.pTriAPI->Vertex3fv( vecPoint );

	gEngfuncs.pTriAPI->TexCoord2f( 1.0f, 0.0f );
	vecPoint = vecOrigin + vecUp * flSpriteSize;
	gEngfuncs.pTriAPI->Vertex3fv( vecPoint );

	gEngfuncs.pTriAPI->TexCoord2f( 0.0f, 0.0f );
	vecPoint = vecOrigin + vecRight * flSpriteSize + vecUp * flSpriteSize;
	gEngfuncs.pTriAPI->Vertex3fv( vecPoint );

	gEngfuncs.pTriAPI->TexCoord2f( 0.0f, 1.0f );
	vecPoint = vecOrigin + vecRight * flSpriteSize;
	gEngfuncs.pTriAPI->Vertex3fv( vecPoint );

	gEngfuncs.pTriAPI->End();
	gEngfuncs.pTriAPI->RenderMode( kRenderTransAdd );
}
