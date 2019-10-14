#include "random.h"

namespace aux
{
	static const uint64 magick1 = 0x5deece66d;
	static const uint64 magick2 = 0xb;
	static const uint32 seedBits = 48;
	static const uint64 seedMask = ((const uint64)0x1 << seedBits) - 1;
	static const uint64 defaultSeed = 0x5555555555555555;

	///////////////////////////////////////////////////////////
	//
	//	Helper functions
	//
	///////////////////////////////////////////////////////////

	static uint32 LL_GetNext(uint64& seed, uint32 bits)
	{
		seed = (seed * magick1 + magick2) & seedMask;
		return (uint32)(seed >> (seedBits - bits));
	}

	///////////////////////////////////////////////////////////
	//
	//	Random functions
	//
	///////////////////////////////////////////////////////////

	Random::Random()
	{
		seed = defaultSeed;
	}

	Random::Random(uint64 seed_)
	{
		seed = seed_;
	}

	int32 Random::GetNext(int32 minimum, int32 maximum)
	{
		return 0;
	}

	float32 Random::GetNext(float32 minimum, float32 maximum)
	{
		return 0.0f;
	}

	uint32 Random::GetNext(uint32 maximum)
	{
		AUX_DEBUG_ASSERT(maximum > 0);

		if (maximum < UINT32_MAX)
		{
			return LL_GetNext(seed, 32) % maximum;
		}

		return LL_GetNext(seed, 32);
	}
}
