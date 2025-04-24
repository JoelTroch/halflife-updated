#pragma once

// Polymorphic Memory Resource was added in C++-17. However, C++-17 is not fully supported by GNU G++ 9 (the highest we
// can use in Steam Runtime version 1 aka "Scout" without additional risky setup) and PMR is sadly one of them (support
// was added since GNU G++ 10, clang 7 and MSVC 16.5 aka Visual Studio 2019).
//
// For the older toolchains, we have no choice but to use standard allocation instead. "std::allocator" was considered
// but the fact that the alignment is "hardcoded and cannot be changed" there made it impossible to use.
#if defined(__cpp_lib_memory_resource)
#define USE_PMR 1
#else
#define USE_PMR 0
#endif

#include <cstddef>
#if USE_PMR
#include <memory_resource>
#endif
#include <vector>

class CBaseParticle;

#define TRIANGLE_FPS 30

/**
*	@brief Simple allocator that uses a chunk-based pool to serve requests.
*/
class CMiniMem
{
private:
	static inline CMiniMem* _instance = nullptr;

#if USE_PMR
	std::pmr::unsynchronized_pool_resource _pool;
#endif

	std::vector<CBaseParticle*> _particles;
	std::size_t _visibleParticles = 0;

protected:
	// private constructor and destructor.
	CMiniMem() = default;
	~CMiniMem() = default;

public:
	void* Allocate(std::size_t sizeInBytes, std::size_t alignment = alignof(std::max_align_t));

	void Deallocate(void* memory, std::size_t sizeInBytes, std::size_t alignment = alignof(std::max_align_t));

	void ProcessAll(); //Processes all

	void Reset(); //clears memory, setting all particles to not used.

	void Shutdown();

	int ApplyForce(Vector vOrigin, Vector vDirection, float flRadius, float flStrength);

	static CMiniMem* Instance();

	std::size_t GetTotalParticles() { return _particles.size(); }
	std::size_t GetDrawnParticles() { return _visibleParticles; }
};
