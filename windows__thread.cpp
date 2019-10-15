#include "thread.h"

#pragma warning(push, 0)

#define WIN32_LEAN_AND_MEAN
#define STRICT
#include <windows.h>

#pragma warning(pop)

namespace aux
{
	struct thread_state_t
	{
		void* user_ptr;
		thread_handler_t handler;
	};

	struct thread_t
	{
		HANDLE handle;
	};

	///////////////////////////////////////////////////////////
	//
	//	Helper functions
	//
	///////////////////////////////////////////////////////////

	static DWORD get_timeout(u32_t msec)
	{
		if (msec == 0)
		{
			return 1;
		}

		if ((DWORD)msec == INFINITE)
		{
			--msec;
		}

		return (DWORD)msec;
	}

	__declspec(nothrow) static DWORD WINAPI on_thread(LPVOID param)
	{
		thread_state_t state = *(thread_state_t*)param;
		free_mem(param);
		return (DWORD)state.handler(state.user_ptr);
	}

	///////////////////////////////////////////////////////////
	//
	//	Thread functions
	//
	///////////////////////////////////////////////////////////

	thread_t* start_thread(thread_handler_t handler, void* user_ptr)
	{
		AUX_DEBUG_ASSERT(handler != nullptr);

		thread_state_t* state = (thread_state_t*)alloc_mem(sizeof(thread_state_t));
		state->handler = handler;
		state->user_ptr = user_ptr;
		HANDLE handle = CreateThread(nullptr, 0, &on_thread, state, CREATE_SUSPENDED, nullptr);

		if (handle != nullptr)
		{
			if (ResumeThread(handle) != (DWORD)-1)
			{
				thread_t* thread = (thread_t*)alloc_mem(sizeof(thread_t));
				thread->handle = handle;
				return thread;
			}

			CloseHandle(handle);
		}

		free_mem(state);
		return nullptr;
	}

	void free_thread(thread_t* thread)
	{
		CloseHandle(thread->handle);
		free_mem(thread);
	}

	void wait_thread(thread_t* thread)
	{
		WaitForSingleObject(thread->handle, INFINITE);
	}

	bool wait_thread(thread_t* thread, u32_t timeout_msec)
	{
		return WaitForSingleObject(thread->handle, get_timeout(timeout_msec)) == WAIT_OBJECT_0;
	}

	void suspend_current_thread(u32_t duration_msec)
	{
		Sleep(get_timeout(duration_msec));
	}
}
