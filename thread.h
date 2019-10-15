#pragma once

#include "base.h"

namespace aux
{
	struct thread_t;
	typedef i32_t(*thread_handler_t)(void* user_ptr);

	thread_t* start_thread(thread_handler_t handler, void* user_ptr = nullptr);
	void free_thread(thread_t* thread);

	void wait_thread(thread_t* thread);
	bool wait_thread(thread_t* thread, u32_t timeout_msec);

	void suspend_current_thread(u32_t duration_msec);
}
