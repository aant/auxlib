#pragma once

#include "base.h"

namespace aux
{
	class Random
	{
	public:

		uint64 seed;

		Random();

		Random(const Random&) = default;
		Random& operator = (const Random&) = default;

		explicit Random(uint64 seed);

		int32 GetNext(int32 minimum, int32 maximum);
		float32 GetNext(float32 minimum, float32 maximum);
		uint32 GetNext(uint32 maximum);
	};
}
