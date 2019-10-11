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
#define AUX_DEBUG_ERROR(message) aux::report_error__SHOULD_NOT_BE_USED_DIRECTLY(L ## message)
#define AUX_DEBUG_ASSERT(expression) if (!(expression)) AUX_DEBUG_ERROR("Expression failed: (" #expression ")")
#define AUX_DEBUG_MEMGUARD_BEGIN() aux::begin_mem_guard__SHOULD_NOT_BE_USED_DIRECTLY()
#define AUX_DEBUG_MEMGUARD_END() aux::end_mem_guard__SHOULD_NOT_BE_USED_DIRECTLY()
#else
#define AUX_DEBUG_ERROR(message) AUX_BLANK_CODE
#define AUX_DEBUG_ASSERT(expression) AUX_BLANK_CODE
#define AUX_DEBUG_MEMGUARD_BEGIN() AUX_BLANK_CODE
#define AUX_DEBUG_MEMGUARD_END() AUX_BLANK_CODE
#endif

namespace aux
{
	typedef int8_t i8_t;
	typedef int16_t i16_t;
	typedef int32_t i32_t;
	typedef int64_t i64_t;
	typedef uint8_t u8_t;
	typedef uint16_t u16_t;
	typedef uint32_t u32_t;
	typedef uint64_t u64_t;
	typedef float f32_t;
	typedef double f64_t;
	typedef int32_t e32_t;

	#if defined(AUX_DEBUG_ON)
	void report_error__SHOULD_NOT_BE_USED_DIRECTLY(const wchar_t message[]);
	void begin_mem_guard__SHOULD_NOT_BE_USED_DIRECTLY();
	void end_mem_guard__SHOULD_NOT_BE_USED_DIRECTLY();
	#endif

	template<typename T>
	inline T min_of(T lhs, T rhs)
	{
		return (lhs < rhs) ? lhs : rhs;
	}

	template<typename T>
	inline T max_of(T lhs, T rhs)
	{
		return (lhs > rhs) ? lhs : rhs;
	}

	template<>
	f32_t min_of<f32_t>(f32_t lhs, f32_t rhs);
	template<>
	f64_t min_of<f64_t>(f64_t lhs, f64_t rhs);

	template<>
	f32_t max_of<f32_t>(f32_t lhs, f32_t rhs);
	template<>
	f64_t max_of<f64_t>(f64_t lhs, f64_t rhs);

	template<typename T>
	inline T clamp(T value, T bound_min, T bound_max)
	{
		AUX_DEBUG_ASSERT(bound_min < bound_max);

		if (value < bound_min)
		{
			return bound_min;
		}

		if (value > bound_max)
		{
			return bound_max;
		}

		return value;
	}

	void* alloc_mem(size_t size);
	void* zalloc_mem(size_t size);
	void free_mem(void* mem);
	void copy_mem(const void* mem_src, void* mem_dst, size_t size);
	void move_mem(const void* mem_src, void* mem_dst, size_t size);
	void fill_mem(void* mem, u8_t value, size_t size);
	void zero_mem(void* mem, size_t size);
	i32_t compare_mem(const void* mem1, const void* mem2, size_t size);
}
