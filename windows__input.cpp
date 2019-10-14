#include "input.h"

#pragma warning(push, 0)

#define WIN32_LEAN_AND_MEAN
#define STRICT
#include <windows.h>

#pragma warning(pop)

namespace aux
{
	enum
	{
		HIDUsagePage_Generic = 0x01,
	};

	enum
	{
		HIDUsage_GenericMouse = 0x02,
		HIDUsage_GenericJoystick = 0x04,
		HIDUsage_GenericGamepad = 0x05,
		HIDUsage_GenericKeyboard = 0x06,
		HIDUsage_GenericKeypad = 0x07,
	};

	enum
	{
		Indicator_CapsLock,
		Indicator_ScrollLock,
		Indicator_NumLock,

		Indicator_MaxEnums
	};

	static const int32 Cursor_MaxDimension = 256;

	#pragma pack(1)

	struct RGBA
	{
		uint8 r;
		uint8 g;
		uint8 b;
		uint8 a;
	};

	struct CursorImpl
	{
		HCURSOR handle;
	};

	struct Input
	{
		HWND window;
		uint8 keys[Key_MaxEnums];
		uint8 indicators[Indicator_MaxEnums];
		Point2 mousePos;
		HCURSOR defaultCursor;
		HCURSOR currentCursor;
		UINT rawBufferSize;
		void* rawBuffer;
		InputCaps caps;
	};

	#pragma pack()

	static Input* input = nullptr;
	InputHandler inputHandler = {};

	///////////////////////////////////////////////////////////
	//
	//	Helper functions
	//
	///////////////////////////////////////////////////////////

	static bool LL_RegisterDevice(HWND window, USHORT page, USHORT usage, DWORD flags)
	{
		RAWINPUTDEVICE desc = {};
		desc.hwndTarget = window;
		desc.usUsagePage = page;
		desc.usUsage = usage;
		desc.dwFlags = flags;
		return (bool)RegisterRawInputDevices(&desc, 1, sizeof(RAWINPUTDEVICE));
	}

	#if 0
	static void LL_UnregisterDevice(USHORT page, USHORT usage)
	{
		RAWINPUTDEVICE desc = {};
		desc.usUsagePage = page;
		desc.usUsage = usage;
		desc.dwFlags = RIDEV_REMOVE;
		RegisterRawInputDevices(&desc, 1, sizeof(RAWINPUTDEVICE));
	}
	#endif

	static enum32 LL_TranslateKey(int32 vKey, USHORT scanCode, USHORT flags)
	{
		switch (vKey)
		{
			case VK_LBUTTON:
				return Key_MouseLeft;
			case VK_RBUTTON:
				return Key_MouseRight;
			case VK_CANCEL:
				return Key_Break;
			case VK_MBUTTON:
				return Key_MouseMiddle;
			case VK_XBUTTON1:
				return Key_MouseX1;
			case VK_XBUTTON2:
				return Key_MouseX2;
			case VK_BACK:
				return Key_Backspace;
			case VK_TAB:
				return Key_Tab;
			case VK_CLEAR:
				return Key_NumClear;
			case VK_RETURN:
				return ((flags & RI_KEY_E0) != 0) ? Key_NumEnter : Key_Enter;
			case VK_SHIFT:
				return (scanCode == 0x2a) ? Key_ShiftLeft : Key_ShiftRight;
			case VK_CONTROL:
				return ((flags & RI_KEY_E0) != 0) ? Key_ControlRight : Key_ControlLeft;
			case VK_MENU:
				return ((flags & RI_KEY_E0) != 0) ? Key_AltRight : Key_AltLeft;
			case VK_PAUSE:
				return Key_Pause;
			case VK_CAPITAL:
				return Key_CapsLock;
			case VK_ESCAPE:
				return Key_Escape;
			case VK_SPACE:
				return Key_Space;
			case VK_PRIOR:
				return Key_PageUp;
			case VK_NEXT:
				return Key_PageDown;
			case VK_END:
				return Key_End;
			case VK_HOME:
				return Key_Home;
			case VK_LEFT:
				return Key_ArrowLeft;
			case VK_UP:
				return Key_ArrowUp;
			case VK_RIGHT:
				return Key_ArrowRight;
			case VK_DOWN:
				return Key_ArrowDown;
			case VK_SNAPSHOT:
				return ((flags & RI_KEY_E0) != 0) ? Key_Snapshot : Key_SysReq;
			case VK_INSERT:
				return Key_Insert;
			case VK_DELETE:
				return Key_Delete;
			case 0x30:
				return Key_0;
			case 0x31:
				return Key_1;
			case 0x32:
				return Key_2;
			case 0x33:
				return Key_3;
			case 0x34:
				return Key_4;
			case 0x35:
				return Key_5;
			case 0x36:
				return Key_6;
			case 0x37:
				return Key_7;
			case 0x38:
				return Key_8;
			case 0x39:
				return Key_9;
			case 0x41:
				return Key_A;
			case 0x42:
				return Key_B;
			case 0x43:
				return Key_C;
			case 0x44:
				return Key_D;
			case 0x45:
				return Key_E;
			case 0x46:
				return Key_F;
			case 0x47:
				return Key_G;
			case 0x48:
				return Key_H;
			case 0x49:
				return Key_I;
			case 0x4a:
				return Key_J;
			case 0x4b:
				return Key_K;
			case 0x4c:
				return Key_L;
			case 0x4d:
				return Key_M;
			case 0x4e:
				return Key_N;
			case 0x4f:
				return Key_O;
			case 0x50:
				return Key_P;
			case 0x51:
				return Key_Q;
			case 0x52:
				return Key_R;
			case 0x53:
				return Key_S;
			case 0x54:
				return Key_T;
			case 0x55:
				return Key_U;
			case 0x56:
				return Key_V;
			case 0x57:
				return Key_W;
			case 0x58:
				return Key_X;
			case 0x59:
				return Key_Y;
			case 0x5a:
				return Key_Z;
			case VK_LWIN:
				return Key_CommandLeft;
			case VK_RWIN:
				return Key_CommandRight;
			case VK_APPS:
				return Key_Application;
			case VK_NUMPAD0:
				return Key_Num0;
			case VK_NUMPAD1:
				return Key_Num1;
			case VK_NUMPAD2:
				return Key_Num2;
			case VK_NUMPAD3:
				return Key_Num3;
			case VK_NUMPAD4:
				return Key_Num4;
			case VK_NUMPAD5:
				return Key_Num5;
			case VK_NUMPAD6:
				return Key_Num6;
			case VK_NUMPAD7:
				return Key_Num7;
			case VK_NUMPAD8:
				return Key_Num8;
			case VK_NUMPAD9:
				return Key_Num9;
			case VK_MULTIPLY:
				return Key_NumMultiply;
			case VK_ADD:
				return Key_NumPlus;
			case VK_SUBTRACT:
				return Key_NumMinus;
			case VK_DECIMAL:
				return Key_NumDecimal;
			case VK_DIVIDE:
				return Key_NumDivide;
			case VK_F1:
				return Key_F1;
			case VK_F2:
				return Key_F2;
			case VK_F3:
				return Key_F3;
			case VK_F4:
				return Key_F4;
			case VK_F5:
				return Key_F5;
			case VK_F6:
				return Key_F6;
			case VK_F7:
				return Key_F7;
			case VK_F8:
				return Key_F8;
			case VK_F9:
				return Key_F9;
			case VK_F10:
				return Key_F10;
			case VK_F11:
				return Key_F11;
			case VK_F12:
				return Key_F12;
			case VK_NUMLOCK:
				return Key_NumLock;
			case VK_SCROLL:
				return Key_ScrollLock;
			case VK_OEM_1:
				return Key_Semicolon;
			case VK_OEM_PLUS:
				return Key_Equals;
			case VK_OEM_COMMA:
				return Key_Comma;
			case VK_OEM_MINUS:
				return Key_Minus;
			case VK_OEM_PERIOD:
				return Key_Period;
			case VK_OEM_2:
				return Key_Slash;
			case VK_OEM_3:
				return Key_Backtick;
			case VK_OEM_4:
				return Key_BracketLeft;
			case VK_OEM_5:
				return Key_Backslash;
			case VK_OEM_6:
				return Key_BracketRight;
			case VK_OEM_7:
				return Key_Apostrophe;
			case VK_OEM_102:
				return Key_Oem102;
			default:
				return -1;
		}
	}

	static uint8 LL_GetIndicatorState(int32 vKey)
	{
		return (uint8)(GetKeyState(vKey) & 0x0001);
	}

	static void LL_ToggleIndicator(enum32 indicator)
	{
		input->indicators[indicator] = (uint8)((input->indicators[indicator] + 1) & 1);
	}

	static void LL_InitIndicators()
	{
		input->indicators[Indicator_CapsLock] = LL_GetIndicatorState(VK_CAPITAL);
		input->indicators[Indicator_ScrollLock] = LL_GetIndicatorState(VK_SCROLL);
		input->indicators[Indicator_NumLock] = LL_GetIndicatorState(VK_NUMLOCK);
	}

	static void LL_InitMouse()
	{
		POINT pos;

		if (GetCursorPos(&pos) && ScreenToClient(input->window, &pos))
		{
			input->mousePos = {(int32)pos.x, (int32)pos.y};
		}
	}

	static void LL_PressKey(int32 vKey, USHORT scanCode, USHORT flags)
	{
		enum32 key = LL_TranslateKey(vKey, scanCode, flags);

		if (key < 0)
		{
			return;
		}

		if (input->keys[key] == 0)
		{
			input->keys[key] = 1;

			switch (key)
			{
				case Key_CapsLock:
					LL_ToggleIndicator(Indicator_CapsLock);
					break;
				case Key_ScrollLock:
					LL_ToggleIndicator(Indicator_ScrollLock);
					break;
				case Key_NumLock:
					LL_ToggleIndicator(Indicator_NumLock);
					break;
			}

			if (inputHandler.OnKeyDown != nullptr)
			{
				inputHandler.OnKeyDown(inputHandler.userPtr, key);
			}
		}
	}

	static void LL_ReleaseKey(int32 vKey, USHORT scanCode, USHORT flags)
	{
		enum32 key = LL_TranslateKey(vKey, scanCode, flags);

		if (key < 0)
		{
			return;
		}

		if (input->keys[key] != 0)
		{
			input->keys[key] = 0;

			if (inputHandler.OnKeyUp != nullptr)
			{
				inputHandler.OnKeyUp(inputHandler.userPtr, key);
			}
		}
	}

	static void LL_ProcessRawKeyboard(const RAWKEYBOARD& raw)
	{
		if (raw.MakeCode != KEYBOARD_OVERRUN_MAKE_CODE)
		{
			if ((raw.Flags & RI_KEY_BREAK) == 0)
			{
				LL_PressKey(raw.VKey, raw.MakeCode, raw.Flags);
			}
			else
			{
				LL_ReleaseKey(raw.VKey, raw.MakeCode, raw.Flags);
			}
		}
	}

	static void LL_ProcessRawMouse(const RAWMOUSE& raw)
	{
		(void)raw;
	}

	static void LL_ProcessRawHID(const RAWHID& raw)
	{
		(void)raw;
	}

	static void LL_RGBtoBGR(const RGBA& src, RGBA& dst)
	{
		dst = {src.b, src.g, src.r, src.a};
	}

	static HBITMAP LL_CreateCursorColorMap(const Size2& size, const void* data)
	{
		BITMAPINFOHEADER info = {};
		info.biSize = sizeof(BITMAPINFOHEADER);
		info.biBitCount = 32;
		info.biWidth = (LONG)size.x;
		info.biHeight = -(LONG)size.y;
		info.biPlanes = 1;
		info.biCompression = BI_RGB;
		void* bitmapData;
		HBITMAP bitmap = CreateDIBSection(nullptr, (const BITMAPINFO*)&info, DIB_RGB_COLORS, &bitmapData, nullptr, 0);

		if (bitmap == nullptr)
		{
			return nullptr;
		}

		int32 pixelCount = size.x * size.y;

		// Copy data (RGB to BGR swapped)
		for (int32 i = 0; i < pixelCount; ++i)
		{
			LL_RGBtoBGR(*((const RGBA*)data + i), *((RGBA*)bitmapData + i));
		}

		// Prevent color inversion
		RGBA& first = *(RGBA*)bitmapData;

		if (first.a == 0)
		{
			first.a = 1;
		}

		return bitmap;
	}

	static HBITMAP LL_CreateCursorMaskMap(const Size2& size)
	{
		int32 pitch = (size.x + 15) / 16;
		size_t dataSize = (size_t)pitch * (size_t)size.y;
		void* temp = Memory_Allocate(dataSize);
		Memory_Fill(temp, 0xff, dataSize);
		HBITMAP bitmap = CreateBitmap(size.x, size.y, 1, 1, temp);
		Memory_Free(temp);
		return bitmap;
	}

	static HCURSOR LL_CreateCursor(HBITMAP colorMap, HBITMAP maskMap, const Point2& hotSpot)
	{
		ICONINFO info = {};
		info.fIcon = FALSE;
		info.hbmColor = colorMap;
		info.hbmMask = maskMap;
		info.xHotspot = (DWORD)hotSpot.x;
		info.yHotspot = (DWORD)hotSpot.y;
		return (HCURSOR)CreateIconIndirect(&info);
	}

	static void LL_SetCurrentCursor(HCURSOR cursor)
	{
		if (input->currentCursor != cursor)
		{
			input->currentCursor = cursor;
			SetCursor(cursor);
		}
	}

	///////////////////////////////////////////////////////////
	//
	//	Internal functions
	//
	///////////////////////////////////////////////////////////

	bool Internal__InitInput(HWND window, HCURSOR defaultCursor)
	{
		input = (Input*)Memory_AllocateAndZero(sizeof(Input));
		input->window = window;
		input->rawBufferSize = sizeof(RAWINPUT);
		input->rawBuffer = Memory_Allocate(sizeof(RAWINPUT));
		input->defaultCursor = defaultCursor;
		input->currentCursor = defaultCursor;

		if (!LL_RegisterDevice(window, HIDUsagePage_Generic, HIDUsage_GenericKeyboard, RIDEV_NOLEGACY | RIDEV_NOHOTKEYS | RIDEV_APPKEYS))
		{
			return false;
		}

		input->caps.maxCursorDimension = Cursor_MaxDimension;
		return true;
	}

	void Internal__FreeInput()
	{
		if (input != nullptr)
		{
			Memory_Free(input->rawBuffer);
			Memory_Free(input);
			input = nullptr;
		}
	}

	void Internal__CaptureInputFocus()
	{
		Memory_Zero(input->keys, sizeof(input->keys));
		LL_InitIndicators();
		LL_InitMouse();
	}

	void Internal__ReleaseInputFocus()
	{
	}

	void Internal__PressVirtualKey(int32 vKey)
	{
		LL_PressKey(vKey, 0, 0);
	}

	void Internal__ReleaseVirtualKey(int32 vKey)
	{
		LL_ReleaseKey(vKey, 0, 0);
	}

	void Internal__DoubleClick(int32 vKey)
	{
		enum32 key = LL_TranslateKey(vKey, 0, 0);

		if (key < 0)
		{
			return;
		}

		if (input->keys[key] == 0)
		{
			input->keys[key] = 1;

			if (inputHandler.OnDoubleClick != nullptr)
			{
				inputHandler.OnDoubleClick(inputHandler.userPtr, key);
			}
		}
	}

	void Internal__MoveMouse(int32 x, int32 y)
	{
		Point2& pos = input->mousePos;

		if ((pos.x != x) || (pos.y != y))
		{
			pos = {x, y};

			if (inputHandler.OnMouseMove != nullptr)
			{
				inputHandler.OnMouseMove(inputHandler.userPtr, pos);
			}
		}
	}

	void Internal__ScrollMouse(int32 pos)
	{
		if (pos != 0)
		{
			if (inputHandler.OnMouseScroll != nullptr)
			{
				inputHandler.OnMouseScroll(inputHandler.userPtr, pos);
			}
		}
	}

	void Internal__ProcessRawInput(WPARAM wParam, LPARAM lParam)
	{
		if (GET_RAWINPUT_CODE_WPARAM(wParam) != RIM_INPUT)
		{
			return;
		}

		UINT size;

		if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, nullptr, &size, sizeof(RAWINPUTHEADER)) == (UINT)-1)
		{
			return;
		}

		if (input->rawBufferSize < size)
		{
			Memory_Free(input->rawBuffer);
			input->rawBufferSize = size;
			input->rawBuffer = Memory_Allocate((size_t)size);
		}

		if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, input->rawBuffer, &size, sizeof(RAWINPUTHEADER)) != (UINT)-1)
		{
			const RAWINPUT* raw = (const RAWINPUT*)input->rawBuffer;

			switch (raw->header.dwType)
			{
				case RIM_TYPEMOUSE:
					LL_ProcessRawMouse(raw->data.mouse);
					break;
				case RIM_TYPEKEYBOARD:
					LL_ProcessRawKeyboard(raw->data.keyboard);
					break;
				case RIM_TYPEHID:
					LL_ProcessRawHID(raw->data.hid);
					break;
			}
		}
	}

	void Internal__UpdateCursor()
	{
		SetCursor(input->currentCursor);
	}

	///////////////////////////////////////////////////////////
	//
	//	Input functions
	//
	///////////////////////////////////////////////////////////

	const InputCaps& Input_GetCaps()
	{
		return input->caps;
	}

	bool Input_IsKeyDown(enum32 key)
	{
		AUX_DEBUG_ASSERT((key >= 0) && (key < Key_MaxEnums));

		return (bool)input->keys[key];
	}

	bool Input_IsKeyOn(enum32 key)
	{
		switch (key)
		{
			case Key_CapsLock:
				return input->indicators[Indicator_CapsLock] != 0;
			case Key_ScrollLock:
				return input->indicators[Indicator_ScrollLock] != 0;
			case Key_NumLock:
				return input->indicators[Indicator_NumLock] != 0;
			default:
				return false;
		}
	}

	const Point2& Input_GetMousePosition()
	{
		return input->mousePos;
	}

	void Input_PlaceMouse(const Point2& position)
	{
		POINT pos = {(LONG)position.x, (LONG)position.y};

		if (ClientToScreen(input->window, &pos))
		{
			SetCursorPos((int)pos.x, (int)pos.y);
		}
	}

	CursorHandle Input_CreateCursor(const Size2& size, const Point2& hotSpot, const void* data)
	{
		AUX_DEBUG_ASSERT(size.x > 0);
		AUX_DEBUG_ASSERT(size.y > 0);
		AUX_DEBUG_ASSERT((hotSpot.x >= 0) && (hotSpot.x < size.x));
		AUX_DEBUG_ASSERT((hotSpot.y >= 0) && (hotSpot.y < size.y));
		AUX_DEBUG_ASSERT(data != nullptr);

		if ((size.x > Cursor_MaxDimension) || (size.y > Cursor_MaxDimension))
		{
			return nullptr;
		}

		HBITMAP colorMap = LL_CreateCursorColorMap(size, data);

		if (colorMap != nullptr)
		{
			HBITMAP maskMap = LL_CreateCursorMaskMap(size);

			if (maskMap != nullptr)
			{
				HCURSOR handle = LL_CreateCursor(colorMap, maskMap, hotSpot);

				if (handle != nullptr)
				{
					CursorHandle cursor = (CursorHandle)Memory_Allocate(sizeof(CursorImpl));
					cursor->handle = handle;
					DeleteObject(maskMap);
					DeleteObject(colorMap);
					return cursor;
				}

				DeleteObject(maskMap);
			}

			DeleteObject(colorMap);
		}

		return nullptr;
	}

	void Input_DestroyCursor(CursorHandle cursor)
	{
		if (cursor->handle == input->currentCursor)
		{
			LL_SetCurrentCursor(nullptr);
		}

		DestroyCursor(cursor->handle);
		Memory_Free(cursor);
	}

	void Input_SetCursor(CursorHandle cursor)
	{
		LL_SetCurrentCursor(cursor->handle);
	}

	void Input_SetDefaultCursor()
	{
		LL_SetCurrentCursor(input->defaultCursor);
	}
}
