#pragma once

#include "base.h"

namespace aux
{
	struct thread_t;

	thread_t* start_thread(i32_t(*handler)(void* user_ptr), void* user_ptr = nullptr);
	void free_thread(thread_t* thread);

	void wait_thread(thread_t* thread);
	bool wait_thread(thread_t* thread, u32_t timeout_msec);

	void suspend_current_thread(u32_t duration_msec);
}
