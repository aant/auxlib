#pragma once

#include "size2.h"

namespace aux
{
	enum
	{
		APP_STYLE_BAD_ENUM = -1,

		APP_STYLE_FULLSCREEN,
		APP_STYLE_FIXED_WINDOW,
		APP_STYLE_RESIZABLE_WINDOW,

		APP_STYLE_MAX_ENUMS,
	};

	struct app_handler_t
	{
		void* user_ptr;
		bool(*on_init)(void* user_ptr);
		void(*on_free)(void* user_ptr);
		void(*on_sleep)(void* user_ptr);
		void(*on_awake)(void* user_ptr);
		void(*on_update)(void* user_ptr, f32_t dt);
		void(*on_resize)(void* user_ptr, const size2_t& res);
		void(*on_redraw)(void* user_ptr);
	};

	e32_t get_app_style();
	const size2_t& get_app_res();

	void start_app(const char title[], e32_t style, const size2_t& res);
	void close_app();
	void reshape_app(e32_t style, const size2_t& res);

	extern app_handler_t app_handler;
}
