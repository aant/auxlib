#include "thread.h"

#pragma warning(push, 0)

#define WIN32_LEAN_AND_MEAN
#define STRICT
#include <windows.h>

#pragma warning(pop)

namespace aux
{
	struct ThreadState
	{
		ThreadHandler handler;
		void* userPtr;
	};

	struct ThreadImpl
	{
		HANDLE handle;
	};

	///////////////////////////////////////////////////////////
	//
	//	Helper functions
	//
	///////////////////////////////////////////////////////////

	static DWORD LL_GetTimeout(uint32 msec)
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

	static DWORD LL_HandleThread(ThreadState& state)
	{
		__try
		{
			return (DWORD)state.handler(state.userPtr);
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
		}

		return (DWORD)-1;
	}

	__declspec(nothrow) static DWORD WINAPI LL_OnThread(LPVOID param)
	{
		ThreadState state = *(ThreadState*)param;
		Memory_Free(param);

		try
		{
			return LL_HandleThread(state);
		}
		catch (...)
		{
		}

		return (DWORD)-1;
	}

	///////////////////////////////////////////////////////////
	//
	//	Thread functions
	//
	///////////////////////////////////////////////////////////

	ThreadHandle Thread_Start(ThreadHandler handler, void* userPtr)
	{
		AUX_DEBUG_ASSERT(handler != nullptr);

		ThreadState* state = (ThreadState*)Memory_Allocate(sizeof(ThreadState));
		state->handler = handler;
		state->userPtr = userPtr;
		HANDLE handle = CreateThread(nullptr, 0, &LL_OnThread, state, CREATE_SUSPENDED, nullptr);

		if (handle != nullptr)
		{
			if (ResumeThread(handle) != (DWORD)-1)
			{
				ThreadHandle thread = (ThreadHandle)Memory_Allocate(sizeof(ThreadImpl));
				thread->handle = handle;
				return thread;
			}

			CloseHandle(handle);
		}

		Memory_Free(state);
		return nullptr;
	}

	void Thread_Free(ThreadHandle thread)
	{
		CloseHandle(thread->handle);
		Memory_Free(thread);
	}

	void Thread_Wait(ThreadHandle thread)
	{
		WaitForSingleObject(thread->handle, INFINITE);
	}

	bool Thread_Wait(ThreadHandle thread, uint32 timeout)
	{
		return WaitForSingleObject(thread->handle, LL_GetTimeout(timeout)) == WAIT_OBJECT_0;
	}

	void Thread_SuspendCurrent(uint32 duration)
	{
		Sleep(LL_GetTimeout(duration));
	}
}
