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

		i32_t get_next(i32_t minimum, i32_t maximum);
		f32_t get_next(f32_t minimum, f32_t maximum);
		u32_t get_next(u32_t maximum);
	};
}
