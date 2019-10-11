#pragma once

#include "base.h"

namespace aux
{
	class random_t
	{
	public:

		u64_t seed;

		random_t();

		random_t(const random_t&) = default;
		random_t& operator = (const random_t&) = default;

		explicit random_t(u64_t seed);

		i32_t get_next_i(i32_t bound_min, i32_t bound_max);
		u32_t get_next_u(u32_t bound_min, u32_t bound_max);
		f32_t get_next_f(f32_t bound_min, f32_t bound_max);
	};
}
