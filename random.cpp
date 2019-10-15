#include "random.h"

namespace aux
{
	static const u64_t magick1 = 0x5deece66d;
	static const u64_t magick2 = 0xb;
	static const u32_t seed_bits = 48;
	static const u64_t seed_mask = ((const u64_t)0x1 << seed_bits) - 1;
	static const u64_t default_seed = 0x5555555555555555;

	///////////////////////////////////////////////////////////
	//
	//	Helper functions
	//
	///////////////////////////////////////////////////////////

	static u32_t get_next(u64_t& seed, u32_t bits)
	{
		seed = (seed * magick1 + magick2) & seed_mask;
		return (u32_t)(seed >> (seed_bits - bits));
	}

	///////////////////////////////////////////////////////////
	//
	//	Random functions
	//
	///////////////////////////////////////////////////////////

	random_t::random_t()
	{
		seed = default_seed;
	}

	random_t::random_t(u64_t seed_)
	{
		seed = seed_;
	}

	i32_t random_t::get_next(i32_t minimum, i32_t maximum)
	{
		(void)minimum;
		(void)maximum;
		return 0;
	}

	f32_t random_t::get_next(f32_t minimum, f32_t maximum)
	{
		(void)minimum;
		(void)maximum;
		return 0.0f;
	}

	u32_t random_t::get_next(u32_t maximum)
	{
		(void)maximum;
		return 0;
	}
}
