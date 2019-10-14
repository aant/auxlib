#include "application.h"
#include "unicode.h"
#include "point2.h"

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
	static const wchar_t Window_ClassName[] = L"52C61109D294F5D9F2C1557ED617368";

	static const int32 Window_MinClientSize = 240;

	static const DWORD WindowStyle_Fullscreen = WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP;
	static const DWORD WindowStyle_FixedBorder = WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX;
	static const DWORD WindowStyle_ResizableBorder = WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_SYSMENU | WS_CAPTION | WS_SIZEBOX | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;

	#pragma pack(1)

	struct WindowShape
	{
		DWORD style;
		DWORD styleEx;
		Point2 pos;
		Size2 size;
	};

	struct Application
	{
		HINSTANCE instance;
		HWND window;
		HCURSOR defaultCursor;
		DWORD currentTime;
		enum32 style;
		Size2 resolution;
		bool foreground;
		bool active;
		bool skipResize;
		bool closing;
	};

	#pragma pack()

	static Application* application = nullptr;
	ApplicationHandler applicationHandler = {};

	///////////////////////////////////////////////////////////
	//
	//	Internal functions
	//
	///////////////////////////////////////////////////////////

	bool Internal__InitInput(HWND window, HCURSOR defaultCursor);
	void Internal__FreeInput();
	void Internal__CaptureInputFocus();
	void Internal__ReleaseInputFocus();
	void Internal__PressVirtualKey(int32 vKey);
	void Internal__ReleaseVirtualKey(int32 vKey);
	void Internal__DoubleClick(int32 vKey);
	void Internal__MoveMouse(int32 x, int32 y);
	void Internal__ScrollMouse(int32 pos);
	void Internal__ProcessRawInput(WPARAM wParam, LPARAM lParam);
	void Internal__UpdateCursor();

	bool Internal__InitGraphics(HWND window);
	void Internal__FreeGraphics();

	bool Internal__InitAudio(HWND window);
	void Internal__FreeAudio();
	void Internal__SuspendAudioPlayback();
	void Internal__ResumeAudioPlayback();

	///////////////////////////////////////////////////////////
	//
	//	Helper functions
	//
	///////////////////////////////////////////////////////////

	static int32 LL_GetWidth(const RECT& rect)
	{
		return (int32)(rect.right - rect.left);
	}

	static int32 LL_GetHeight(const RECT& rect)
	{
		return (int32)(rect.bottom - rect.top);
	}

	static void LL_ForceReleaseMouse()
	{
		INPUT input = {};
		input.type = INPUT_MOUSE;
		input.mi.dwExtraInfo = (ULONG_PTR)GetMessageExtraInfo();
		input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
		SendInput(1, &input, sizeof(input));
	}

	static void LL_UpdateResolution()
	{
		RECT client;

		if (GetClientRect(application->window, &client))
		{
			application->resolution = {LL_GetWidth(client), LL_GetHeight(client)};
		}
	}

	static void LL_InitSettings(HINSTANCE instance, HWND window, enum32 style)
	{
		application->instance = instance;
		application->window = window;
		application->defaultCursor = LoadCursorW(nullptr, IDC_ARROW);
		application->style = style;
		LL_UpdateResolution();
	}

	static bool LL_RegisterWindowClass(HINSTANCE instance, WNDPROC handler)
	{
		WNDCLASSW wc = {};
		wc.hInstance = instance;
		wc.lpszClassName = Window_ClassName;
		wc.lpfnWndProc = handler;
		wc.hIcon = LoadIconW(nullptr, IDI_WINLOGO);
		wc.style = CS_DBLCLKS;
		return RegisterClassW(&wc) != (ATOM)0;
	}

	static void LL_UnregisterWindowClass()
	{
		UnregisterClassW(Window_ClassName, application->instance);
	}

	static bool LL_CalcWindowShape(enum32 style, const Size2& clientSize, WindowShape& shape)
	{
		switch (style)
		{
			case AppStyle_Fullscreen:
				shape.style = WindowStyle_Fullscreen;
				break;
			case AppStyle_FixedWindow:
				shape.style = WindowStyle_FixedBorder;
				break;
			case AppStyle_ResizableWindow:
				shape.style = WindowStyle_ResizableBorder;
				break;
			default:
				return false;
		}

		shape.styleEx = WS_EX_APPWINDOW;
		const int32 screenWidth = GetSystemMetrics(SM_CXSCREEN);
		const int32 screenHeight = GetSystemMetrics(SM_CYSCREEN);

		if (style == AppStyle_Fullscreen)
		{
			shape.pos = {};
			shape.size = {screenWidth, screenHeight};
			return true;
		}

		RECT bounds;

		if ((clientSize.x == 0) && (clientSize.y == 0))
		{
			bounds = {0, 0, (LONG)screenWidth, (LONG)screenHeight};
		}
		else
		{
			bounds = {0, 0, (LONG)clientSize.x, (LONG)clientSize.y};
		}

		if (AdjustWindowRectEx(&bounds, shape.style, FALSE, shape.styleEx))
		{
			RECT work;

			if (!SystemParametersInfoW(SPI_GETWORKAREA, 0, &work, 0))
			{
				work = {0, 0, (LONG)screenWidth, (LONG)screenHeight};
			}

			const int32 workWidth = LL_GetWidth(work);
			const int32 workHeight = LL_GetHeight(work);
			shape.size.x = Math_Clamp(LL_GetWidth(bounds), Window_MinClientSize, workWidth);
			shape.size.y = Math_Clamp(LL_GetHeight(bounds), Window_MinClientSize, workHeight);
			shape.pos.x = (int32)work.left + (workWidth - shape.size.x) / 2;
			shape.pos.y = (int32)work.top + (workHeight - shape.size.y) / 2;
			return true;
		}

		return false;
	}

	static void LL_CalcWindowMinMax(HWND window, MINMAXINFO& minMax)
	{
		RECT mins = {0, 0, Window_MinClientSize, Window_MinClientSize};
		DWORD style = (DWORD)GetWindowLongW(window, GWL_STYLE);
		DWORD styleEx = (DWORD)GetWindowLongW(window, GWL_EXSTYLE);

		if (AdjustWindowRectEx(&mins, style, GetMenu(window) != nullptr, styleEx))
		{
			minMax.ptMinTrackSize = {(LONG)LL_GetWidth(mins), (LONG)LL_GetHeight(mins)};
		}
	}

	static HWND LL_CreateWindow(const char title[], enum32 style, const Size2& clientSize)
	{
		WindowShape shape;

		if (!LL_CalcWindowShape(style, clientSize, shape))
		{
			return nullptr;
		}

		uint16* title16 = Unicode_Utf8to16((const uint8*)title, true);
		HWND window = CreateWindowExW(shape.styleEx, Window_ClassName, (const wchar_t*)title16, shape.style, shape.pos.x, shape.pos.y, shape.size.x, shape.size.y, nullptr, nullptr, application->instance, nullptr);

		if (title16 != nullptr)
		{
			Unicode_Free(title16);
		}

		return window;
	}

	static void LL_ResizeWindow(HWND window)
	{
		RECT client;

		if (GetClientRect(window, &client))
		{
			int32 clientWidth = (int32)LL_GetWidth(client);
			int32 clientHeight = (int32)LL_GetHeight(client);

			if ((clientWidth > 0) && (clientHeight > 0))
			{
				Size2& res = application->resolution;

				if ((res.x != clientWidth) || (res.y != clientHeight))
				{
					res = {clientWidth, clientHeight};

					if (applicationHandler.OnResize != nullptr)
					{
						applicationHandler.OnResize(applicationHandler.userPtr, res);
					}
				}
			}
		}
	}

	static void LL_ToggleWindowFocus(HWND window, bool active, bool minimized)
	{
		if (active)
		{
			application->foreground = true;
		}
		else
		{
			application->foreground = false;

			if (application->active)
			{
				Internal__SuspendAudioPlayback();
				Internal__ReleaseInputFocus();
				application->active = false;
			}

			if ((application->style == AppStyle_Fullscreen) && !minimized)
			{
				CloseWindow(window);
			}
		}
	}

	static void LL_SuspendMainLoop(HWND window)
	{
		LL_ToggleWindowFocus(window, false, false);
	}

	static void LL_ResumeMainLoop()
	{
		application->foreground = true;
		application->currentTime = timeGetTime();
	}

	static void LL_UpdateMainLoop()
	{
		if (!application->active)
		{
			Internal__CaptureInputFocus();
			Internal__ResumeAudioPlayback();
			application->active = true;
			application->currentTime = timeGetTime();
		}

		DWORD t1 = timeGetTime();
		DWORD dt = t1 - application->currentTime;

		if (dt > 0)
		{
			application->currentTime = t1;
		}

		if (applicationHandler.OnUpdate != nullptr)
		{
			applicationHandler.OnUpdate(applicationHandler.userPtr, (float32)dt / 1000);
		}

		if (applicationHandler.OnRedraw != nullptr)
		{
			applicationHandler.OnRedraw(applicationHandler.userPtr);
		}
	}

	static void LL_RunMainLoop()
	{
		application->currentTime = timeGetTime();
		MSG message;

		while (application->window != nullptr)
		{
			if (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE))
			{
				DispatchMessageW(&message);
			}
			else
			{
				if (application->foreground)
				{
					LL_UpdateMainLoop();
				}
				else
				{
					WaitMessage();
				}
			}
		}
	}

	static LRESULT CALLBACK LL_OnWindowMessage(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
	{
		switch (message)
		{
			case WM_CREATE:
				LL_ForceReleaseMouse();
				break;
			case WM_SIZE:
				if (!application->skipResize)
				{
					LL_ResizeWindow(window);
				}
				break;
			case WM_ACTIVATE:
				LL_ToggleWindowFocus(window, LOWORD(wParam) != WA_INACTIVE, HIWORD(wParam) != 0);
				break;
			case WM_PAINT:
				ValidateRect(window, nullptr);
				return 0;
			case WM_CLOSE:
				application->window = nullptr;
				return 0;
			case WM_ERASEBKGND:
				return 1;
			case WM_SETCURSOR:
				if (application->active && (LOWORD(lParam) == HTCLIENT))
				{
					Internal__UpdateCursor();
					return 1;
				}
				break;
			case WM_GETMINMAXINFO:
				LL_CalcWindowMinMax(window, *(MINMAXINFO*)lParam);
				break;
			case WM_NCHITTEST:
				if (application->style == AppStyle_Fullscreen)
				{
					return HTCLIENT;
				}
				break;
			case WM_INPUT:
				if (application->active)
				{
					Internal__ProcessRawInput(wParam, lParam);
				}
				break;
			case WM_SYSCOMMAND:
				switch (wParam)
				{
					case SC_SCREENSAVE:
					case SC_MONITORPOWER:
						return 0;
				}
				break;
			case WM_MOUSEMOVE:
				if (application->active)
				{
					const Size2& res = application->resolution;
					int32 x = (SHORT)LOWORD(lParam);
					int32 y = (SHORT)HIWORD(lParam);

					if ((x >= 0) && (x < res.x) && (y >= 0) && (y < res.y))
					{
						Internal__MoveMouse(x, y);
					}
				}
				break;
			case WM_LBUTTONDOWN:
				Internal__PressVirtualKey(VK_LBUTTON);
				break;
			case WM_LBUTTONUP:
				Internal__ReleaseVirtualKey(VK_LBUTTON);
				break;
			case WM_LBUTTONDBLCLK:
				Internal__DoubleClick(VK_LBUTTON);
				break;
			case WM_RBUTTONDOWN:
				Internal__PressVirtualKey(VK_RBUTTON);
				break;
			case WM_RBUTTONUP:
				Internal__ReleaseVirtualKey(VK_RBUTTON);
				break;
			case WM_RBUTTONDBLCLK:
				Internal__DoubleClick(VK_RBUTTON);
				break;
			case WM_MBUTTONDOWN:
				Internal__PressVirtualKey(VK_MBUTTON);
				break;
			case WM_MBUTTONUP:
				Internal__ReleaseVirtualKey(VK_MBUTTON);
				break;
			case WM_MBUTTONDBLCLK:
				Internal__DoubleClick(VK_MBUTTON);
				break;
			case WM_MOUSEWHEEL:
				Internal__ScrollMouse((SHORT)HIWORD(wParam) / WHEEL_DELTA);
				break;
			case WM_XBUTTONDOWN:
				switch (HIWORD(wParam))
				{
					case XBUTTON1:
						Internal__PressVirtualKey(VK_XBUTTON1);
						break;
					case XBUTTON2:
						Internal__PressVirtualKey(VK_XBUTTON2);
						break;
				}
				break;
			case WM_XBUTTONUP:
				switch (HIWORD(wParam))
				{
					case XBUTTON1:
						Internal__ReleaseVirtualKey(VK_XBUTTON1);
						break;
					case XBUTTON2:
						Internal__ReleaseVirtualKey(VK_XBUTTON2);
						break;
				}
				break;
			case WM_XBUTTONDBLCLK:
				switch (HIWORD(wParam))
				{
					case XBUTTON1:
						Internal__DoubleClick(VK_XBUTTON1);
						break;
					case XBUTTON2:
						Internal__DoubleClick(VK_XBUTTON2);
						break;
				}
				break;
			case WM_ENTERMENULOOP:
				LL_SuspendMainLoop(window);
				break;
			case WM_EXITMENULOOP:
				LL_ResumeMainLoop();
				break;
			case WM_POWERBROADCAST:
				switch (wParam)
				{
					case PBT_APMRESUMEAUTOMATIC:
						if (applicationHandler.OnAwake != nullptr)
						{
							applicationHandler.OnAwake(applicationHandler.userPtr);
						}
						break;
					case PBT_APMSUSPEND:
						if (applicationHandler.OnSleep != nullptr)
						{
							applicationHandler.OnSleep(applicationHandler.userPtr);
						}
						break;
				}
				break;
			case WM_ENTERSIZEMOVE:
				LL_SuspendMainLoop(window);
				application->skipResize = true;
				break;
			case WM_EXITSIZEMOVE:
				LL_ResizeWindow(window);
				LL_ResumeMainLoop();
				application->skipResize = false;
				break;
		}

		return DefWindowProcW(window, message, wParam, lParam);
	}

	static bool LL_InitSubsystems(HWND window)
	{
		if (!Internal__InitInput(window, application->defaultCursor))
		{
			return false;
		}

		if (!Internal__InitGraphics(window))
		{
			return false;
		}

		if (!Internal__InitAudio(window))
		{
			return false;
		}

		if (applicationHandler.OnInit != nullptr)
		{
			if (!applicationHandler.OnInit(applicationHandler.userPtr))
			{
				return false;
			}
		}

		return true;
	}

	static void LL_FreeSubsystems()
	{
		if (applicationHandler.OnFree != nullptr)
		{
			applicationHandler.OnFree(applicationHandler.userPtr);
		}

		Internal__FreeAudio();
		Internal__FreeGraphics();
		Internal__FreeInput();
	}

	///////////////////////////////////////////////////////////
	//
	//	Application functions
	//
	///////////////////////////////////////////////////////////

	enum32 Application_GetStyle()
	{
		return application->style;
	}

	const Size2& Application_GetResolution()
	{
		return application->resolution;
	}

	void Application_Start(const char title[], enum32 style, const Size2& resolution)
	{
		AUX_DEBUG_ASSERT(application == nullptr);

		application = (Application*)Memory_AllocateAndZero(sizeof(Application));

		if (SUCCEEDED(CoInitializeEx(nullptr, COINIT_MULTITHREADED)))
		{
			MMRESULT timeResult = timeBeginPeriod(1);
			HINSTANCE instance = GetModuleHandleW(nullptr);

			if (LL_RegisterWindowClass(instance, &LL_OnWindowMessage))
			{
				HWND window = LL_CreateWindow(title, style, resolution);

				if (window != nullptr)
				{
					LL_InitSettings(instance, window, style);

					if (LL_InitSubsystems(window))
					{
						ShowWindow(window, SW_SHOWNORMAL);

						if (UpdateWindow(window))
						{
							LL_RunMainLoop();
						}

						ShowWindow(window, SW_HIDE);
					}

					LL_FreeSubsystems();
					DestroyWindow(window);
				}

				LL_UnregisterWindowClass();
			}

			if (timeResult == TIMERR_NOERROR)
			{
				timeEndPeriod(1);
			}

			CoUninitialize();
		}

		Memory_Free(application);
		application = nullptr;
	}

	void Application_Close()
	{
		HWND window = application->window;

		if (!application->closing && (window != nullptr) && IsWindow(window))
		{
			application->closing = true;
			PostMessageW(window, WM_CLOSE, 0, 0);
		}
	}

	void Application_Reshape(enum32 style, const Size2& resolution)
	{
		DWORD flags = SWP_NOZORDER | SWP_ASYNCWINDOWPOS | SWP_SHOWWINDOW;
		bool styleChanged = style != application->style;

		if (styleChanged)
		{
			flags |= SWP_FRAMECHANGED;

			switch (style)
			{
				case AppStyle_Fullscreen:
					break;
				case AppStyle_FixedWindow:
				case AppStyle_ResizableWindow:
					flags |= SWP_DRAWFRAME;
					break;
				default:
					return;
			}
		}

		WindowShape shape;

		if (LL_CalcWindowShape(style, resolution, shape))
		{
			HWND window = application->window;

			if (styleChanged)
			{
				if (SetWindowLongW(window, GWL_STYLE, (LONG)shape.style) == 0)
				{
					return;
				}

				application->style = style;
			}

			if (SetWindowPos(window, nullptr, shape.pos.x, shape.pos.y, shape.size.x, shape.size.y, flags))
			{
				LL_UpdateResolution();
			}
		}
	}
}
