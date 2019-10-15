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

	__declspec(noreturn) static void out_of_mem()
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

	void report_debug_error__SHOULD_NOT_BE_USED_DIRECTLY(const wchar_t message[])
	{
		MessageBoxW(GetForegroundWindow(), message, L"DEBUG ERROR", MB_TOPMOST | MB_TASKMODAL | MB_ICONERROR | MB_OK);
		DebugBreak();
	}

	void begin_debug_memory_guard__SHOULD_NOT_BE_USED_DIRECTLY()
	{
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	}

	void end_debug_memory_guard__SHOULD_NOT_BE_USED_DIRECTLY()
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
	f32_t min_of<f32_t>(f32_t lhs, f32_t rhs)
	{
		return fminf(lhs, rhs);
	}

	template<>
	f64_t min_of<f64_t>(f64_t lhs, f64_t rhs)
	{
		return fmin(lhs, rhs);
	}

	template<>
	f32_t max_of<f32_t>(f32_t lhs, f32_t rhs)
	{
		return fmaxf(lhs, rhs);
	}

	template<>
	f64_t max_of<f64_t>(f64_t lhs, f64_t rhs)
	{
		return fmax(lhs, rhs);
	}

	///////////////////////////////////////////////////////////
	//
	//	Memory functions
	//
	///////////////////////////////////////////////////////////

	void* alloc_mem(size_t size)
	{
		AUX_DEBUG_ASSERT(size > 0);

		void* mem = malloc(size);

		if (mem != nullptr)
		{
			return mem;
		}

		out_of_mem();
	}

	void* zalloc_mem(size_t size)
	{
		AUX_DEBUG_ASSERT(size > 0);

		void* mem = malloc(size);

		if (mem != nullptr)
		{
			memset(mem, 0, size);
			return mem;
		}

		out_of_mem();
	}

	void free_mem(void* mem)
	{
		AUX_DEBUG_ASSERT(mem != nullptr);

		free(mem);
	}

	void copy_mem(const void* mem_src, void* mem_dst, size_t size)
	{
		memcpy(mem_dst, mem_src, size);
	}

	void move_mem(const void* mem_src, void* mem_dst, size_t size)
	{
		memmove(mem_dst, mem_src, size);
	}

	void fill_mem(void* mem, u8_t value, size_t size)
	{
		memset(mem, value, size);
	}

	void zero_mem(void* mem, size_t size)
	{
		memset(mem, 0, size);
	}

	i32_t compare_mem(const void* mem1, const void* mem2, size_t size)
	{
		return (i32_t)memcmp(mem1, mem2, size);
	}
}
