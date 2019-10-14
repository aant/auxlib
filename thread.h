#pragma once

#include "base.h"

namespace aux
{
	typedef struct ThreadImpl* ThreadHandle;
	typedef int32(*ThreadHandler)(void* userPtr);

	ThreadHandle Thread_Start(ThreadHandler handler, void* userPtr = nullptr);
	void Thread_Free(ThreadHandle thread);

	void Thread_Wait(ThreadHandle thread);
	bool Thread_Wait(ThreadHandle thread, uint32 timeout);

	void Thread_SuspendCurrent(uint32 duration);
}
