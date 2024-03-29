#include "application.h"
#include "unicode.h"

#pragma warning(push, 0)

#define WIN32_LEAN_AND_MEAN
#define STRICT
#include <windows.h>
#include <objbase.h>
#include <timeapi.h>

#pragma warning(pop)

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "winmm.lib")

namespace aux
{
	static const wchar_t class_name[] = L"52C61109D294F5D9F2C1557ED617368";

	static const DWORD style_fullscreen = WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP;
	static const DWORD style_fixed_border = WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX;
	static const DWORD style_resizable_border = WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_SYSMENU | WS_CAPTION | WS_SIZEBOX | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;

	static const i32_t min_client_size = 240;

	#pragma pack(1)

	struct window_shape_t
	{
		DWORD style;
		DWORD style_ext;
		i32_t x;
		i32_t y;
		i32_t w;
		i32_t h;
	};

	struct app_t
	{
		HINSTANCE instance;
		HWND window;
		HCURSOR default_cursor;
		DWORD current_time;
		e32_t style;
		size2_t resolution;
		bool foreground;
		bool active;
		bool skip_resize;
		bool closing;
	};

	#pragma pack()

	static app_t* app = nullptr;
	app_handler_t app_handler = {};

	///////////////////////////////////////////////////////////
	//
	//	Internal functions
	//
	///////////////////////////////////////////////////////////

	bool internal__init_input(HWND window, HCURSOR default_cursor);
	void internal__free_input();
	void internal__capture_input_focus();
	void internal__release_input_focus();
	void internal__press_virtual_key(i32_t vkey);
	void internal__release_virtual_key(i32_t vkey);
	void internal__double_click(i32_t vkey);
	void internal__move_mouse(i32_t x, i32_t y);
	void internal__scroll_mouse(i32_t pos);
	void internal__process_raw_input(WPARAM wparam, LPARAM lparam);
	void internal__update_cursor();

	bool internal__init_graphics(HWND window);
	void internal__free_graphics();

	bool internal__init_audio(HWND window);
	void internal__free_audio();
	void internal__suspend_audio_playback();
	void internal__resume_audio_playback();

	///////////////////////////////////////////////////////////
	//
	//	Helper functions
	//
	///////////////////////////////////////////////////////////

	static LONG get_w(const RECT& rect)
	{
		return rect.right - rect.left;
	}

	static LONG get_h(const RECT& rect)
	{
		return rect.bottom - rect.top;
	}

	static void set_rect(RECT& rect, LONG left, LONG top, LONG right, LONG bottom)
	{
		rect.left = left;
		rect.top = top;
		rect.right = right;
		rect.bottom = bottom;
	}

	static void force_release_mouse()
	{
		INPUT input = {};
		input.type = INPUT_MOUSE;
		input.mi.dwExtraInfo = (ULONG_PTR)GetMessageExtraInfo();
		input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
		SendInput(1, &input, sizeof(input));
	}

	static void update_resolution()
	{
		RECT client;

		if (GetClientRect(app->window, &client))
		{
			app->resolution.w = (i32_t)get_w(client);
			app->resolution.h = (i32_t)get_h(client);
		}
	}

	static void init_settings(HINSTANCE instance, HWND window, e32_t style)
	{
		app->instance = instance;
		app->window = window;
		app->default_cursor = LoadCursorW(nullptr, IDC_ARROW);
		app->style = style;
		update_resolution();
	}

	static bool register_window_class(HINSTANCE instance, WNDPROC handler)
	{
		WNDCLASSW wc = {};
		wc.hInstance = instance;
		wc.lpszClassName = class_name;
		wc.lpfnWndProc = handler;
		wc.hIcon = LoadIconW(nullptr, IDI_WINLOGO);
		wc.style = CS_DBLCLKS;
		return RegisterClassW(&wc) != (ATOM)0;
	}

	static void unregister_window_class()
	{
		UnregisterClassW(class_name, app->instance);
	}

	static bool calc_window_shape(e32_t style, const size2_t& client_size, window_shape_t& shape)
	{
		switch (style)
		{
			case APP_STYLE_FULLSCREEN:
				shape.style = style_fullscreen;
				break;
			case APP_STYLE_FIXED_WINDOW:
				shape.style = style_fixed_border;
				break;
			case APP_STYLE_RESIZABLE_WINDOW:
				shape.style = style_resizable_border;
				break;
			default:
				return false;
		}

		shape.style_ext = WS_EX_APPWINDOW;
		const i32_t screen_w = GetSystemMetrics(SM_CXSCREEN);
		const i32_t screen_h = GetSystemMetrics(SM_CYSCREEN);

		if (style == APP_STYLE_FULLSCREEN)
		{
			shape.x = 0;
			shape.y = 0;
			shape.w = screen_w;
			shape.h = screen_h;
			return true;
		}

		RECT bounds;

		if ((client_size.w == 0) && (client_size.h == 0))
		{
			set_rect(bounds, 0, 0, (LONG)screen_w, (LONG)screen_h);
		}
		else
		{
			set_rect(bounds, 0, 0, (LONG)client_size.w, (LONG)client_size.h);
		}

		if (AdjustWindowRectEx(&bounds, shape.style, FALSE, shape.style_ext))
		{
			RECT work;

			if (!SystemParametersInfoW(SPI_GETWORKAREA, 0, &work, 0))
			{
				set_rect(work, 0, 0, (LONG)screen_w, (LONG)screen_h);
			}

			const i32_t work_w = (i32_t)get_w(work);
			const i32_t work_h = (i32_t)get_h(work);
			shape.w = clamp<i32_t>((i32_t)get_w(bounds), min_client_size, work_w);
			shape.h = clamp<i32_t>((i32_t)get_h(bounds), min_client_size, work_h);
			shape.x = (i32_t)work.left + (work_w - shape.w) / 2;
			shape.y = (i32_t)work.top + (work_h - shape.h) / 2;
			return true;
		}

		return false;
	}

	static void calc_window_min_max(HWND window, MINMAXINFO& info)
	{
		RECT mins = {0, 0, min_client_size, min_client_size};
		DWORD style = (DWORD)GetWindowLongW(window, GWL_STYLE);
		DWORD style_ext = (DWORD)GetWindowLongW(window, GWL_EXSTYLE);

		if (AdjustWindowRectEx(&mins, style, GetMenu(window) != nullptr, style_ext))
		{
			info.ptMinTrackSize.x = get_w(mins);
			info.ptMinTrackSize.y = get_h(mins);
		}
	}

	static HWND create_window(const char title[], e32_t style, const size2_t& client_size)
	{
		window_shape_t shape;

		if (!calc_window_shape(style, client_size, shape))
		{
			return nullptr;
		}

		u16_t* title16 = nullptr;

		if (title != nullptr)
		{
			title16 = to_utf16((const u8_t*)title, true);
		}

		HWND window = CreateWindowExW(shape.style_ext, class_name, (const wchar_t*)title16, shape.style, shape.x, shape.y, shape.w, shape.h, nullptr, nullptr, app->instance, nullptr);

		if (title16 != nullptr)
		{
			free_utf(title16);
		}

		return window;
	}

	static void resize_window(HWND window)
	{
		RECT client;

		if (GetClientRect(window, &client))
		{
			i32_t client_w = (i32_t)get_w(client);
			i32_t client_h = (i32_t)get_h(client);

			if ((client_w > 0) && (client_h > 0))
			{
				size2_t& res = app->resolution;

				if ((res.w != client_w) || (res.h != client_h))
				{
					res.w = client_w;
					res.h = client_h;

					if (app_handler.on_resize != nullptr)
					{
						app_handler.on_resize(app_handler.user_ptr, res);
					}
				}
			}
		}
	}

	static void toggle_window_focus(HWND window, bool active, bool minimized)
	{
		if (active)
		{
			app->foreground = true;
		}
		else
		{
			app->foreground = false;

			if (app->active)
			{
				internal__suspend_audio_playback();
				internal__release_input_focus();
				app->active = false;
			}

			if ((app->style == APP_STYLE_FULLSCREEN) && !minimized)
			{
				CloseWindow(window);
			}
		}
	}

	static void suspend_main_loop(HWND window)
	{
		toggle_window_focus(window, false, false);
	}

	static void resume_main_loop()
	{
		app->foreground = true;
		app->current_time = timeGetTime();
	}

	static void update_main_loop()
	{
		if (!app->active)
		{
			internal__capture_input_focus();
			internal__resume_audio_playback();
			app->active = true;
			app->current_time = timeGetTime();
		}

		DWORD t1 = timeGetTime();
		DWORD dt = t1 - app->current_time;

		if (dt > 0)
		{
			app->current_time = t1;
		}

		if (app_handler.on_update != nullptr)
		{
			app_handler.on_update(app_handler.user_ptr, (f32_t)dt / 1000);
		}

		if (app_handler.on_redraw != nullptr)
		{
			app_handler.on_redraw(app_handler.user_ptr);
		}
	}

	static void run_main_loop()
	{
		app->current_time = timeGetTime();
		MSG message;

		while (app->window != nullptr)
		{
			if (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE))
			{
				DispatchMessageW(&message);
			}
			else
			{
				if (app->foreground)
				{
					update_main_loop();
				}
				else
				{
					WaitMessage();
				}
			}
		}
	}

	static LRESULT CALLBACK on_window_message(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
	{
		switch (message)
		{
			case WM_CREATE:
				force_release_mouse();
				break;
			case WM_SIZE:
				if (!app->skip_resize)
				{
					resize_window(window);
				}
				break;
			case WM_ACTIVATE:
				toggle_window_focus(window, LOWORD(wparam) != WA_INACTIVE, HIWORD(wparam) != 0);
				break;
			case WM_PAINT:
				ValidateRect(window, nullptr);
				return 0;
			case WM_CLOSE:
				app->window = nullptr;
				return 0;
			case WM_ERASEBKGND:
				return 1;
			case WM_SETCURSOR:
				if (app->active && (LOWORD(lparam) == HTCLIENT))
				{
					internal__update_cursor();
					return 1;
				}
				break;
			case WM_GETMINMAXINFO:
				calc_window_min_max(window, *(MINMAXINFO*)lparam);
				break;
			case WM_NCHITTEST:
				if (app->style == APP_STYLE_FULLSCREEN)
				{
					return HTCLIENT;
				}
				break;
			case WM_INPUT:
				if (app->active)
				{
					internal__process_raw_input(wparam, lparam);
				}
				break;
			case WM_SYSCOMMAND:
				switch (wparam)
				{
					case SC_SCREENSAVE:
					case SC_MONITORPOWER:
						return 0;
				}
				break;
			case WM_MOUSEMOVE:
				if (app->active)
				{
					const size2_t& res = app->resolution;
					i32_t w = (SHORT)LOWORD(lparam);
					i32_t h = (SHORT)HIWORD(lparam);

					if ((w >= 0) && (w < res.w) && (h >= 0) && (h < res.h))
					{
						internal__move_mouse(w, h);
					}
				}
				break;
			case WM_LBUTTONDOWN:
				internal__press_virtual_key(VK_LBUTTON);
				break;
			case WM_LBUTTONUP:
				internal__release_virtual_key(VK_LBUTTON);
				break;
			case WM_LBUTTONDBLCLK:
				internal__double_click(VK_LBUTTON);
				break;
			case WM_RBUTTONDOWN:
				internal__press_virtual_key(VK_RBUTTON);
				break;
			case WM_RBUTTONUP:
				internal__release_virtual_key(VK_RBUTTON);
				break;
			case WM_RBUTTONDBLCLK:
				internal__double_click(VK_RBUTTON);
				break;
			case WM_MBUTTONDOWN:
				internal__press_virtual_key(VK_MBUTTON);
				break;
			case WM_MBUTTONUP:
				internal__release_virtual_key(VK_MBUTTON);
				break;
			case WM_MBUTTONDBLCLK:
				internal__double_click(VK_MBUTTON);
				break;
			case WM_MOUSEWHEEL:
				internal__scroll_mouse((SHORT)HIWORD(wparam) / WHEEL_DELTA);
				break;
			case WM_XBUTTONDOWN:
				switch (HIWORD(wparam))
				{
					case XBUTTON1:
						internal__press_virtual_key(VK_XBUTTON1);
						break;
					case XBUTTON2:
						internal__press_virtual_key(VK_XBUTTON2);
						break;
				}
				break;
			case WM_XBUTTONUP:
				switch (HIWORD(wparam))
				{
					case XBUTTON1:
						internal__release_virtual_key(VK_XBUTTON1);
						break;
					case XBUTTON2:
						internal__release_virtual_key(VK_XBUTTON2);
						break;
				}
				break;
			case WM_XBUTTONDBLCLK:
				switch (HIWORD(wparam))
				{
					case XBUTTON1:
						internal__double_click(VK_XBUTTON1);
						break;
					case XBUTTON2:
						internal__double_click(VK_XBUTTON2);
						break;
				}
				break;
			case WM_ENTERMENULOOP:
				suspend_main_loop(window);
				break;
			case WM_EXITMENULOOP:
				resume_main_loop();
				break;
			case WM_POWERBROADCAST:
				switch (wparam)
				{
					case PBT_APMRESUMEAUTOMATIC:
						if (app_handler.on_awake != nullptr)
						{
							app_handler.on_awake(app_handler.user_ptr);
						}
						break;
					case PBT_APMSUSPEND:
						if (app_handler.on_sleep != nullptr)
						{
							app_handler.on_sleep(app_handler.user_ptr);
						}
						break;
				}
				break;
			case WM_ENTERSIZEMOVE:
				suspend_main_loop(window);
				app->skip_resize = true;
				break;
			case WM_EXITSIZEMOVE:
				resize_window(window);
				resume_main_loop();
				app->skip_resize = false;
				break;
		}

		return DefWindowProcW(window, message, wparam, lparam);
	}

	static bool init_subsystems(HWND window)
	{
		if (!internal__init_input(window, app->default_cursor))
		{
			return false;
		}

		if (!internal__init_graphics(window))
		{
			return false;
		}

		if (!internal__init_audio(window))
		{
			return false;
		}

		if (app_handler.on_init != nullptr)
		{
			if (!app_handler.on_init(app_handler.user_ptr))
			{
				return false;
			}
		}

		return true;
	}

	static void free_subsystems()
	{
		if (app_handler.on_free != nullptr)
		{
			app_handler.on_free(app_handler.user_ptr);
		}

		internal__free_audio();
		internal__free_graphics();
		internal__free_input();
	}

	///////////////////////////////////////////////////////////
	//
	//	Application functions
	//
	///////////////////////////////////////////////////////////

	e32_t get_app_style()
	{
		return app->style;
	}

	const size2_t& get_app_res()
	{
		return app->resolution;
	}

	void start_app(const char title[], e32_t style, const size2_t& res)
	{
		AUX_DEBUG_ASSERT(app == nullptr);

		app = (app_t*)zalloc_mem(sizeof(app_t));

		if (SUCCEEDED(CoInitializeEx(nullptr, COINIT_MULTITHREADED)))
		{
			MMRESULT time_result = timeBeginPeriod(1);
			HINSTANCE instance = GetModuleHandleW(nullptr);

			if (register_window_class(instance, &on_window_message))
			{
				HWND window = create_window(title, style, res);

				if (window != nullptr)
				{
					init_settings(instance, window, style);

					if (init_subsystems(window))
					{
						ShowWindow(window, SW_SHOWNORMAL);

						if (UpdateWindow(window))
						{
							run_main_loop();
						}

						ShowWindow(window, SW_HIDE);
					}

					free_subsystems();
					DestroyWindow(window);
				}

				unregister_window_class();
			}

			if (time_result == TIMERR_NOERROR)
			{
				timeEndPeriod(1);
			}

			CoUninitialize();
		}

		free_mem(app);
		app = nullptr;
	}

	void close_app()
	{
		HWND window = app->window;

		if (!app->closing && (window != nullptr) && IsWindow(window))
		{
			app->closing = true;
			PostMessageW(window, WM_CLOSE, 0, 0);
		}
	}

	void reshape_app(e32_t style, const size2_t& res)
	{
		HWND window = app->window;
		DWORD flags = SWP_NOZORDER | SWP_ASYNCWINDOWPOS | SWP_SHOWWINDOW;
		bool style_changed = style != app->style;

		if (style_changed)
		{
			flags |= SWP_FRAMECHANGED;

			switch (style)
			{
				case APP_STYLE_FULLSCREEN:
					break;
				case APP_STYLE_FIXED_WINDOW:
				case APP_STYLE_RESIZABLE_WINDOW:
					flags |= SWP_DRAWFRAME;
					break;
				default:
					return;
			}
		}

		window_shape_t shape;

		if (calc_window_shape(style, res, shape))
		{
			if (style_changed)
			{
				if (SetWindowLongW(window, GWL_STYLE, (LONG)shape.style) == 0)
				{
					return;
				}

				app->style = style;
			}

			if (SetWindowPos(window, nullptr, shape.x, shape.y, shape.w, shape.h, flags))
			{
				update_resolution();
			}
		}
	}
}
