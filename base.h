#pragma once

#include <stddef.h>
#include <stdint.h>

#if defined(AUX_BLANK_CODE)
#pragma message "AUX_BLANK_CODE is already defined"
#endif

#if defined(AUX_DEBUG_ON)
#pragma message "AUX_DEBUG_ON is already defined"
#endif

#if defined(AUX_DEBUG_ERROR)
#pragma message "AUX_DEBUG_ERROR is already defined"
#endif

#if defined(AUX_DEBUG_ASSERT)
#pragma message "AUX_DEBUG_ASSERT is already defined"
#endif

#if defined(AUX_DEBUG_MEMGUARD_BEGIN)
#pragma message "AUX_DEBUG_MEMGUARD_BEGIN is already defined"
#endif

#if defined(AUX_DEBUG_MEMGUARD_END)
#pragma message "AUX_DEBUG_MEMGUARD_END is already defined"
#endif

#define AUX_BLANK_CODE ((void)0)

#if defined(_DEBUG)
#define AUX_DEBUG_ON
#endif

#if defined(AUX_DEBUG_ON)
#define AUX_DEBUG_ERROR(message) aux::Debug_ReportError__SHOULD_NOT_BE_USED_DIRECTLY(L ## message)
#define AUX_DEBUG_ASSERT(expression) if (!(expression)) AUX_DEBUG_ERROR("Expression failed: (" #expression ")")
#define AUX_DEBUG_MEMGUARD_BEGIN() aux::Debug_BeginMemoryGuard__SHOULD_NOT_BE_USED_DIRECTLY()
#define AUX_DEBUG_MEMGUARD_END() aux::Debug_EndMemoryGuard__SHOULD_NOT_BE_USED_DIRECTLY()
#else
#define AUX_DEBUG_ERROR(message) AUX_BLANK_CODE
#define AUX_DEBUG_ASSERT(expression) AUX_BLANK_CODE
#define AUX_DEBUG_MEMGUARD_BEGIN() AUX_BLANK_CODE
#define AUX_DEBUG_MEMGUARD_END() AUX_BLANK_CODE
#endif

namespace aux
{
	typedef int8_t int8;
	typedef int16_t int16;
	typedef int32_t int32;
	typedef int64_t int64;
	typedef uint8_t uint8;
	typedef uint16_t uint16;
	typedef uint32_t uint32;
	typedef uint64_t uint64;
	typedef float float32;
	typedef double float64;
	typedef int32_t enum32;

	const float64 Math_Pi = 3.1415926535897932384626433832795;
	const float64 Math_E = 2.7182818284590452353602874713527;

	#if defined(AUX_DEBUG_ON)
	void Debug_ReportError__SHOULD_NOT_BE_USED_DIRECTLY(const wchar_t message[]);
	void Debug_BeginMemoryGuard__SHOULD_NOT_BE_USED_DIRECTLY();
	void Debug_EndMemoryGuard__SHOULD_NOT_BE_USED_DIRECTLY();
	#endif

	template<typename T>
	inline T Math_Min(T lhs, T rhs)
	{
		return (lhs < rhs) ? lhs : rhs;
	}

	template<typename T>
	inline T Math_Max(T lhs, T rhs)
	{
		return (lhs > rhs) ? lhs : rhs;
	}

	template<>
	float32 Math_Min<float32>(float32 lhs, float32 rhs);
	template<>
	float64 Math_Min<float64>(float64 lhs, float64 rhs);

	template<>
	float32 Math_Max<float32>(float32 lhs, float32 rhs);
	template<>
	float64 Math_Max<float64>(float64 lhs, float64 rhs);

	template<typename T>
	inline T Math_Clamp(T value, T minimum, T maximum)
	{
		AUX_DEBUG_ASSERT(minimum < maximum);

		if (value < minimum)
		{
			return minimum;
		}

		if (value > maximum)
		{
			return maximum;
		}

		return value;
	}

	int8 Math_Abs(int8 value);
	int16 Math_Abs(int16 value);
	int32 Math_Abs(int32 value);
	int64 Math_Abs(int64 value);
	float32 Math_Abs(float32 value);
	float64 Math_Abs(float64 value);

	float32 Math_Trunc(float32 value);
	float64 Math_Trunc(float64 value);
	float32 Math_Floor(float32 value);
	float64 Math_Floor(float64 value);
	float32 Math_Ceil(float32 value);
	float64 Math_Ceil(float64 value);
	float32 Math_Round(float32 value);
	float64 Math_Round(float64 value);

	float32 Math_Sqrt(float32 value);
	float64 Math_Sqrt(float64 value);

	float32 Math_Rad(float32 degrees);
	float64 Math_Rad(float64 degrees);
	float32 Math_Deg(float32 radians);
	float64 Math_Deg(float64 radians);

	float32 Math_Sin(float32 radians);
	float64 Math_Sin(float64 radians);
	float32 Math_Cos(float32 radians);
	float64 Math_Cos(float64 radians);
	float32 Math_Tan(float32 radians);
	float64 Math_Tan(float64 radians);

	float32 Math_Asin(float32 value);
	float64 Math_Asin(float64 value);
	float32 Math_Acos(float32 value);
	float64 Math_Acos(float64 value);
	float32 Math_Atan(float32 value);
	float64 Math_Atan(float64 value);

	void* Memory_Allocate(size_t size);
	void* Memory_AllocateAndZero(size_t size);
	void Memory_Free(void* mem);
	void Memory_Copy(const void* memSrc, void* memDst, size_t size);
	void Memory_Move(const void* memSrc, void* memDst, size_t size);
	void Memory_Fill(void* mem, uint8 value, size_t size);
	void Memory_Zero(void* mem, size_t size);
	int32 Memory_Compare(const void* mem1, const void* mem2, size_t size);
}
