#include "base.h"

#pragma warning(push, 0)

#include <stdlib.h>
#include <math.h>

#define WIN32_LEAN_AND_MEAN
#define STRICT
#include <windows.h>

#if defined(AUX_DEBUG_ON)
#include <crtdbg.h>
#endif

#pragma warning(pop)

namespace aux
{
	///////////////////////////////////////////////////////////
	//
	//	Helper functions
	//
	///////////////////////////////////////////////////////////

	__declspec(noreturn) static void LL_OutOfMem()
	{
		MessageBoxW(GetForegroundWindow(), L"Out of system memory.", L"CRITICAL ERROR", MB_TOPMOST | MB_TASKMODAL | MB_ICONERROR | MB_OK);
		ExitProcess((UINT)-1);
	}

	///////////////////////////////////////////////////////////
	//
	//	Debug functions
	//
	///////////////////////////////////////////////////////////

	#if defined(AUX_DEBUG_ON)

	void Debug_ReportError__SHOULD_NOT_BE_USED_DIRECTLY(const wchar_t message[])
	{
		MessageBoxW(GetForegroundWindow(), message, L"DEBUG ERROR", MB_TOPMOST | MB_TASKMODAL | MB_ICONERROR | MB_OK);
		DebugBreak();
	}

	void Debug_BeginMemoryGuard__SHOULD_NOT_BE_USED_DIRECTLY()
	{
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	}

	void Debug_EndMemoryGuard__SHOULD_NOT_BE_USED_DIRECTLY()
	{
		_CrtDumpMemoryLeaks();
	}

	#endif

	///////////////////////////////////////////////////////////
	//
	//	Math functions
	//
	///////////////////////////////////////////////////////////

	template<>
	float32 Math_Min<float32>(float32 lhs, float32 rhs)
	{
		return fminf(lhs, rhs);
	}

	template<>
	float64 Math_Min<float64>(float64 lhs, float64 rhs)
	{
		return fmin(lhs, rhs);
	}

	template<>
	float32 Math_Max<float32>(float32 lhs, float32 rhs)
	{
		return fmaxf(lhs, rhs);
	}

	template<>
	float64 Math_Max<float64>(float64 lhs, float64 rhs)
	{
		return fmax(lhs, rhs);
	}

	int8 Math_Abs(int8 value)
	{
		return (int8)abs(value);
	}

	int16 Math_Abs(int16 value)
	{
		return (int16)abs(value);
	}

	int32 Math_Abs(int32 value)
	{
		return abs(value);
	}

	int64 Math_Abs(int64 value)
	{
		return llabs(value);
	}

	float32 Math_Abs(float32 value)
	{
		return fabsf(value);
	}

	float64 Math_Abs(float64 value)
	{
		return fabs(value);
	}

	float32 Math_Trunc(float32 value)
	{
		return truncf(value);
	}

	float64 Math_Trunc(float64 value)
	{
		return trunc(value);
	}

	float32 Math_Floor(float32 value)
	{
		return floorf(value);
	}

	float64 Math_Floor(float64 value)
	{
		return floor(value);
	}

	float32 Math_Ceil(float32 value)
	{
		return ceilf(value);
	}

	float64 Math_Ceil(float64 value)
	{
		return ceil(value);
	}

	float32 Math_Round(float32 value)
	{
		return roundf(value);
	}

	float64 Math_Round(float64 value)
	{
		return round(value);
	}

	float32 Math_Sqrt(float32 value)
	{
		return sqrtf(value);
	}

	float64 Math_Sqrt(float64 value)
	{
		return sqrt(value);
	}

	float32 Math_Rad(float32 degrees)
	{
		return degrees * ((const float32)Math_Pi / 180.0f);
	}

	float64 Math_Rad(float64 degrees)
	{
		return degrees * (Math_Pi / 180.0);
	}

	float32 Math_Deg(float32 radians)
	{
		return radians * (180.0f / (const float32)Math_Pi);
	}

	float64 Math_Deg(float64 radians)
	{
		return radians * (180.0 / Math_Pi);
	}

	float32 Math_Sin(float32 radians)
	{
		return sinf(radians);
	}

	float64 Math_Sin(float64 radians)
	{
		return sin(radians);
	}

	float32 Math_Cos(float32 radians)
	{
		return cosf(radians);
	}

	float64 Math_Cos(float64 radians)
	{
		return cos(radians);
	}

	float32 Math_Tan(float32 radians)
	{
		return tanf(radians);
	}

	float64 Math_Tan(float64 radians)
	{
		return tan(radians);
	}

	float32 Math_Asin(float32 value)
	{
		return asinf(value);
	}

	float64 Math_Asin(float64 value)
	{
		return asin(value);
	}

	float32 Math_Acos(float32 value)
	{
		return acosf(value);
	}

	float64 Math_Acos(float64 value)
	{
		return acos(value);
	}

	float32 Math_Atan(float32 value)
	{
		return atanf(value);
	}

	float64 Math_Atan(float64 value)
	{
		return atan(value);
	}

	///////////////////////////////////////////////////////////
	//
	//	Memory functions
	//
	///////////////////////////////////////////////////////////

	void* Memory_Allocate(size_t size)
	{
		AUX_DEBUG_ASSERT(size > 0);

		void* mem = malloc(size);

		if (mem != nullptr)
		{
			return mem;
		}

		LL_OutOfMem();
	}

	void* Memory_AllocateAndZero(size_t size)
	{
		AUX_DEBUG_ASSERT(size > 0);

		void* mem = malloc(size);

		if (mem != nullptr)
		{
			memset(mem, 0, size);
			return mem;
		}

		LL_OutOfMem();
	}

	void Memory_Free(void* mem)
	{
		AUX_DEBUG_ASSERT(mem != nullptr);

		free(mem);
	}

	void Memory_Copy(const void* memSrc, void* memDst, size_t size)
	{
		memcpy(memDst, memSrc, size);
	}

	void Memory_Move(const void* memSrc, void* memDst, size_t size)
	{
		memmove(memDst, memSrc, size);
	}

	void Memory_Fill(void* mem, uint8 value, size_t size)
	{
		memset(mem, value, size);
	}

	void Memory_Zero(void* mem, size_t size)
	{
		memset(mem, 0, size);
	}

	int32 Memory_Compare(const void* mem1, const void* mem2, size_t size)
	{
		return (int32)memcmp(mem1, mem2, size);
	}
}
