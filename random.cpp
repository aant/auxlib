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

	static i32_t next_i(u64_t& seed, u32_t bits)
	{
		seed = (seed * magick1 + magick2) & seed_mask;
		return (i32_t)(seed >> (seed_bits - bits));
	}

	static u32_t next_u(u64_t& seed, u32_t bits)
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

	i32_t random_t::get_next_i(i32_t bound_min, i32_t bound_max)
	{
		return 0;
	}

	u32_t random_t::get_next_u(u32_t bound_min, u32_t bound_max)
	{
		return 0;
	}

	f32_t random_t::get_next_f(f32_t bound_min, f32_t bound_max)
	{
		return 0.0f;
	}
}
