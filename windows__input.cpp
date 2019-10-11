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
		HID_USAGE_PAGE_GENERIC = 0x01,
	};

	enum
	{
		HID_USAGE_GENERIC_MOUSE = 0x02,
		HID_USAGE_GENERIC_JOYSTICK = 0x04,
		HID_USAGE_GENERIC_GAMEPAD = 0x05,
		HID_USAGE_GENERIC_KEYBOARD = 0x06,
		HID_USAGE_GENERIC_KEYPAD = 0x07,
	};

	enum
	{
		MAX_CURSOR_DIM = 256,
	};

	enum
	{
		INDICATOR_BAD_ENUM = -1,

		INDICATOR_CAPS_LOCK,
		INDICATOR_SCROLL_LOCK,
		INDICATOR_NUMPAD_LOCK,

		INDICATOR_MAX_ENUMS
	};

	#pragma pack(1)

	struct rgba_t
	{
		u8_t r;
		u8_t g;
		u8_t b;
		u8_t a;
	};

	struct cursor_t
	{
		HCURSOR handle;
	};

	struct input_t
	{
		HWND window;
		u8_t keys[KEY_MAX_ENUMS];
		u8_t indicators[INDICATOR_MAX_ENUMS];
		point2_t mouse_pos;
		HCURSOR default_cursor;
		HCURSOR current_cursor;
		UINT raw_buffer_size;
		void* raw_buffer;
	};

	#pragma pack()

	static input_t* input = nullptr;
	input_handler_t input_handler = {};

	///////////////////////////////////////////////////////////
	//
	//	Helper functions
	//
	///////////////////////////////////////////////////////////

	static bool register_device(HWND window, USHORT page, USHORT usage, DWORD flags)
	{
		RAWINPUTDEVICE desc = {};
		desc.hwndTarget = window;
		desc.usUsagePage = page;
		desc.usUsage = usage;
		desc.dwFlags = flags;
		return (bool)RegisterRawInputDevices(&desc, 1, sizeof(RAWINPUTDEVICE));
	}

	#if 0
	static void unregister_device(USHORT page, USHORT usage)
	{
		RAWINPUTDEVICE desc = {};
		desc.usUsagePage = page;
		desc.usUsage = usage;
		desc.dwFlags = RIDEV_REMOVE;
		RegisterRawInputDevices(&desc, 1, sizeof(RAWINPUTDEVICE));
	}
	#endif

	static e32_t translate_key(i32_t vkey, USHORT scan_code, USHORT flags)
	{
		switch (vkey)
		{
			case VK_LBUTTON:
				return KEY_MOUSE_LEFT;
			case VK_RBUTTON:
				return KEY_MOUSE_RIGHT;
			case VK_CANCEL:
				return KEY_BREAK;
			case VK_MBUTTON:
				return KEY_MOUSE_MIDDLE;
			case VK_XBUTTON1:
				return KEY_MOUSE_X1;
			case VK_XBUTTON2:
				return KEY_MOUSE_X2;
			case VK_BACK:
				return KEY_BACKSPACE;
			case VK_TAB:
				return KEY_TAB;
			case VK_CLEAR:
				return KEY_NUM_CLEAR;
			case VK_RETURN:
				return ((flags & RI_KEY_E0) != 0) ? KEY_NUM_ENTER : KEY_ENTER;
			case VK_SHIFT:
				return (scan_code == 0x2a) ? KEY_SHIFT_LEFT : KEY_SHIFT_RIGHT;
			case VK_CONTROL:
				return ((flags & RI_KEY_E0) != 0) ? KEY_CONTROL_RIGHT : KEY_CONTROL_LEFT;
			case VK_MENU:
				return ((flags & RI_KEY_E0) != 0) ? KEY_ALT_RIGHT : KEY_ALT_LEFT;
			case VK_PAUSE:
				return KEY_PAUSE;
			case VK_CAPITAL:
				return KEY_CAPS_LOCK;
			case VK_ESCAPE:
				return KEY_ESCAPE;
			case VK_SPACE:
				return KEY_SPACE;
			case VK_PRIOR:
				return KEY_PAGE_UP;
			case VK_NEXT:
				return KEY_PAGE_DOWN;
			case VK_END:
				return KEY_END;
			case VK_HOME:
				return KEY_HOME;
			case VK_LEFT:
				return KEY_ARROW_LEFT;
			case VK_UP:
				return KEY_ARROW_UP;
			case VK_RIGHT:
				return KEY_ARROW_RIGHT;
			case VK_DOWN:
				return KEY_ARROW_DOWN;
			case VK_SNAPSHOT:
				return ((flags & RI_KEY_E0) != 0) ? KEY_SNAPSHOT : KEY_SYSRQ;
			case VK_INSERT:
				return KEY_INSERT;
			case VK_DELETE:
				return KEY_DELETE;
			case 0x30:
				return KEY_0;
			case 0x31:
				return KEY_1;
			case 0x32:
				return KEY_2;
			case 0x33:
				return KEY_3;
			case 0x34:
				return KEY_4;
			case 0x35:
				return KEY_5;
			case 0x36:
				return KEY_6;
			case 0x37:
				return KEY_7;
			case 0x38:
				return KEY_8;
			case 0x39:
				return KEY_9;
			case 0x41:
				return KEY_A;
			case 0x42:
				return KEY_B;
			case 0x43:
				return KEY_C;
			case 0x44:
				return KEY_D;
			case 0x45:
				return KEY_E;
			case 0x46:
				return KEY_F;
			case 0x47:
				return KEY_G;
			case 0x48:
				return KEY_H;
			case 0x49:
				return KEY_I;
			case 0x4a:
				return KEY_J;
			case 0x4b:
				return KEY_K;
			case 0x4c:
				return KEY_L;
			case 0x4d:
				return KEY_M;
			case 0x4e:
				return KEY_N;
			case 0x4f:
				return KEY_O;
			case 0x50:
				return KEY_P;
			case 0x51:
				return KEY_Q;
			case 0x52:
				return KEY_R;
			case 0x53:
				return KEY_S;
			case 0x54:
				return KEY_T;
			case 0x55:
				return KEY_U;
			case 0x56:
				return KEY_V;
			case 0x57:
				return KEY_W;
			case 0x58:
				return KEY_X;
			case 0x59:
				return KEY_Y;
			case 0x5a:
				return KEY_Z;
			case VK_LWIN:
				return KEY_COMMAND_LEFT;
			case VK_RWIN:
				return KEY_COMMAND_RIGHT;
			case VK_APPS:
				return KEY_APPLICATION;
			case VK_NUMPAD0:
				return KEY_NUM0;
			case VK_NUMPAD1:
				return KEY_NUM1;
			case VK_NUMPAD2:
				return KEY_NUM2;
			case VK_NUMPAD3:
				return KEY_NUM3;
			case VK_NUMPAD4:
				return KEY_NUM4;
			case VK_NUMPAD5:
				return KEY_NUM5;
			case VK_NUMPAD6:
				return KEY_NUM6;
			case VK_NUMPAD7:
				return KEY_NUM7;
			case VK_NUMPAD8:
				return KEY_NUM8;
			case VK_NUMPAD9:
				return KEY_NUM9;
			case VK_MULTIPLY:
				return KEY_NUM_MULTIPLY;
			case VK_ADD:
				return KEY_NUM_PLUS;
			case VK_SUBTRACT:
				return KEY_NUM_MINUS;
			case VK_DECIMAL:
				return KEY_NUM_DECIMAL;
			case VK_DIVIDE:
				return KEY_NUM_DIVIDE;
			case VK_F1:
				return KEY_F1;
			case VK_F2:
				return KEY_F2;
			case VK_F3:
				return KEY_F3;
			case VK_F4:
				return KEY_F4;
			case VK_F5:
				return KEY_F5;
			case VK_F6:
				return KEY_F6;
			case VK_F7:
				return KEY_F7;
			case VK_F8:
				return KEY_F8;
			case VK_F9:
				return KEY_F9;
			case VK_F10:
				return KEY_F10;
			case VK_F11:
				return KEY_F11;
			case VK_F12:
				return KEY_F12;
			case VK_NUMLOCK:
				return KEY_NUM_LOCK;
			case VK_SCROLL:
				return KEY_SCROLL_LOCK;
			case VK_LMENU:
				return KEY_COMMAND_LEFT;
			case VK_RMENU:
				return KEY_COMMAND_RIGHT;
			case VK_OEM_1:
				return KEY_SEMICOLON;
			case VK_OEM_PLUS:
				return KEY_EQUALS;
			case VK_OEM_COMMA:
				return KEY_COMMA;
			case VK_OEM_MINUS:
				return KEY_MINUS;
			case VK_OEM_PERIOD:
				return KEY_PERIOD;
			case VK_OEM_2:
				return KEY_SLASH;
			case VK_OEM_3:
				return KEY_BACKTICK;
			case VK_OEM_4:
				return KEY_BRACKET_LEFT;
			case VK_OEM_5:
				return KEY_BACKSLASH;
			case VK_OEM_6:
				return KEY_BRACKET_RIGHT;
			case VK_OEM_7:
				return KEY_APOSTROPHE;
			case VK_OEM_102:
				return KEY_OEM102;
			default:
				return -1;
		}
	}

	static u8_t get_indicator_state(i32_t vkey)
	{
		return (u8_t)(GetKeyState(vkey) & 0x0001);
	}

	static void toggle_indicator(e32_t indicator)
	{
		input->indicators[indicator] = (u8_t)((input->indicators[indicator] + 1) & 1);
	}

	static void init_indicators()
	{
		input->indicators[INDICATOR_CAPS_LOCK] = get_indicator_state(VK_CAPITAL);
		input->indicators[INDICATOR_SCROLL_LOCK] = get_indicator_state(VK_SCROLL);
		input->indicators[INDICATOR_NUMPAD_LOCK] = get_indicator_state(VK_NUMLOCK);
	}

	static void init_mouse()
	{
		POINT pos;

		if (GetCursorPos(&pos) && ScreenToClient(input->window, &pos))
		{
			input->mouse_pos.x = (i32_t)pos.x;
			input->mouse_pos.y = (i32_t)pos.y;
		}
	}

	static void press_key(i32_t vkey, USHORT scan_code, USHORT flags)
	{
		e32_t k = translate_key(vkey, scan_code, flags);

		if (k < 0)
		{
			return;
		}

		if (input->keys[k] == 0)
		{
			input->keys[k] = 1;

			switch (k)
			{
				case KEY_CAPS_LOCK:
					toggle_indicator(INDICATOR_CAPS_LOCK);
					break;
				case KEY_SCROLL_LOCK:
					toggle_indicator(INDICATOR_SCROLL_LOCK);
					break;
				case KEY_NUM_LOCK:
					toggle_indicator(INDICATOR_NUMPAD_LOCK);
					break;
			}

			if (input_handler.on_key_down != nullptr)
			{
				input_handler.on_key_down(input_handler.user_ptr, k);
			}
		}
	}

	static void release_key(i32_t vkey, USHORT scan_code, USHORT flags)
	{
		e32_t k = translate_key(vkey, scan_code, flags);

		if (k < 0)
		{
			return;
		}

		if (input->keys[k] != 0)
		{
			input->keys[k] = 0;

			if (input_handler.on_key_up != nullptr)
			{
				input_handler.on_key_up(input_handler.user_ptr, k);
			}
		}
	}

	static void process_raw_keyboard(const RAWKEYBOARD& raw)
	{
		if (raw.MakeCode != KEYBOARD_OVERRUN_MAKE_CODE)
		{
			if ((raw.Flags & RI_KEY_BREAK) == 0)
			{
				press_key(raw.VKey, raw.MakeCode, raw.Flags);
			}
			else
			{
				release_key(raw.VKey, raw.MakeCode, raw.Flags);
			}
		}
	}

	static void process_raw_mouse(const RAWMOUSE& raw)
	{
		(void)raw;
	}

	static void process_raw_hid(const RAWHID& raw)
	{
		(void)raw;
	}

	static void rgb_to_bgr(const rgba_t& src, rgba_t& dst)
	{
		dst.r = src.b;
		dst.g = src.g;
		dst.b = src.r;
		dst.a = src.a;
	}

	static HBITMAP create_cursor_color_map(i32_t width, i32_t height, const void* data)
	{
		BITMAPINFOHEADER info = {};
		info.biSize = sizeof(BITMAPINFOHEADER);
		info.biBitCount = 32;
		info.biWidth = (LONG)width;
		info.biHeight = -(LONG)height;
		info.biPlanes = 1;
		info.biCompression = BI_RGB;
		void* bitmap_data;
		HBITMAP bitmap = CreateDIBSection(nullptr, (const BITMAPINFO*)&info, DIB_RGB_COLORS, &bitmap_data, nullptr, 0);

		if (bitmap == nullptr)
		{
			return nullptr;
		}

		i32_t pixel_count = width * height;

		// Copy data (RGB to BGR swapped)
		for (i32_t i = 0; i < pixel_count; ++i)
		{
			rgb_to_bgr(*((const rgba_t*)data + i), *((rgba_t*)bitmap_data + i));
		}

		// Prevent color inversion
		rgba_t& first = *(rgba_t*)bitmap_data;

		if (first.a == 0)
		{
			first.a = 1;
		}

		return bitmap;
	}

	static HBITMAP create_cursor_mask_map(i32_t width, i32_t height)
	{
		i32_t pitch = (width + 15) / 16;
		size_t size = (size_t)pitch * (size_t)height;
		void* temp = alloc_mem(size);
		fill_mem(temp, 0xff, size);
		HBITMAP bitmap = CreateBitmap(width, height, 1, 1, temp);
		free_mem(temp);
		return bitmap;
	}

	static HCURSOR create_cursor_object(HBITMAP color_map, HBITMAP mask_map, i32_t hot_x, i32_t hot_y)
	{
		ICONINFO info = {};
		info.fIcon = FALSE;
		info.hbmColor = color_map;
		info.hbmMask = mask_map;
		info.xHotspot = (DWORD)hot_x;
		info.yHotspot = (DWORD)hot_y;
		return (HCURSOR)CreateIconIndirect(&info);
	}

	static void set_current_cursor(HCURSOR cursor)
	{
		if (input->current_cursor != cursor)
		{
			input->current_cursor = cursor;
			SetCursor(cursor);
		}
	}

	///////////////////////////////////////////////////////////
	//
	//	Internal functions
	//
	///////////////////////////////////////////////////////////

	bool internal__init_input(HWND window, HCURSOR default_cursor)
	{
		input = (input_t*)zalloc_mem(sizeof(input_t));
		input->window = window;
		input->raw_buffer_size = sizeof(RAWINPUT);
		input->raw_buffer = alloc_mem(sizeof(RAWINPUT));
		input->default_cursor = default_cursor;
		input->current_cursor = default_cursor;

		if (!register_device(window, HID_USAGE_PAGE_GENERIC, HID_USAGE_GENERIC_KEYBOARD, RIDEV_NOLEGACY | RIDEV_NOHOTKEYS | RIDEV_APPKEYS))
		{
			return false;
		}

		return true;
	}

	void internal__free_input()
	{
		if (input != nullptr)
		{
			free_mem(input->raw_buffer);
			free_mem(input);
			input = nullptr;
		}
	}

	void internal__capture_input_focus()
	{
		zero_mem(input->keys, sizeof(input->keys));
		init_indicators();
		init_mouse();
	}

	void internal__release_input_focus()
	{
	}

	void internal__press_virtual_key(i32_t vkey)
	{
		press_key(vkey, 0, 0);
	}

	void internal__release_virtual_key(i32_t vkey)
	{
		release_key(vkey, 0, 0);
	}

	void internal__double_click(i32_t vkey)
	{
		e32_t k = translate_key(vkey, 0, 0);

		if (k < 0)
		{
			return;
		}

		if (input->keys[k] == 0)
		{
			input->keys[k] = 1;

			if (input_handler.on_double_click != nullptr)
			{
				input_handler.on_double_click(input_handler.user_ptr, k);
			}
		}
	}

	void internal__move_mouse(i32_t x, i32_t y)
	{
		point2_t& pos = input->mouse_pos;

		if ((pos.x != x) || (pos.y != y))
		{
			pos.x = x;
			pos.y = y;

			if (input_handler.on_mouse_move != nullptr)
			{
				input_handler.on_mouse_move(input_handler.user_ptr, pos);
			}
		}
	}

	void internal__scroll_mouse(i32_t pos)
	{
		if (pos != 0)
		{
			if (input_handler.on_mouse_scroll != nullptr)
			{
				input_handler.on_mouse_scroll(input_handler.user_ptr, pos);
			}
		}
	}

	void internal__process_raw_input(WPARAM wp, LPARAM lp)
	{
		if (GET_RAWINPUT_CODE_WPARAM(wp) != RIM_INPUT)
		{
			return;
		}

		UINT size;

		if (GetRawInputData((HRAWINPUT)lp, RID_INPUT, nullptr, &size, sizeof(RAWINPUTHEADER)) == (UINT)-1)
		{
			return;
		}

		if (input->raw_buffer_size < size)
		{
			free_mem(input->raw_buffer);
			input->raw_buffer_size = size;
			input->raw_buffer = alloc_mem((size_t)size);
		}

		if (GetRawInputData((HRAWINPUT)lp, RID_INPUT, input->raw_buffer, &size, sizeof(RAWINPUTHEADER)) != (UINT)-1)
		{
			const RAWINPUT* raw = (const RAWINPUT*)input->raw_buffer;

			switch (raw->header.dwType)
			{
				case RIM_TYPEMOUSE:
					process_raw_mouse(raw->data.mouse);
					break;
				case RIM_TYPEKEYBOARD:
					process_raw_keyboard(raw->data.keyboard);
					break;
				case RIM_TYPEHID:
					process_raw_hid(raw->data.hid);
					break;
			}
		}
	}

	void internal__update_cursor()
	{
		SetCursor(input->current_cursor);
	}

	///////////////////////////////////////////////////////////
	//
	//	Input functions
	//
	///////////////////////////////////////////////////////////

	const input_caps_t& get_input_caps()
	{
		static input_caps_t temp = {};
		return temp;
	}

	bool is_key_down(e32_t key)
	{
		AUX_DEBUG_ASSERT((key >= 0) && (key < KEY_MAX_ENUMS));

		return (bool)input->keys[key];
	}

	bool is_key_on(e32_t key)
	{
		switch (key)
		{
			case KEY_CAPS_LOCK:
				return input->indicators[INDICATOR_CAPS_LOCK] != 0;
			case KEY_SCROLL_LOCK:
				return input->indicators[INDICATOR_SCROLL_LOCK] != 0;
			case KEY_NUM_LOCK:
				return input->indicators[INDICATOR_NUMPAD_LOCK] != 0;
			default:
				return false;
		}
	}

	const point2_t& get_mouse_pos()
	{
		return input->mouse_pos;
	}

	void set_mouse_pos(i32_t x, i32_t y)
	{
		POINT pos;
		pos.x = (LONG)x;
		pos.y = (LONG)y;

		if (ClientToScreen(input->window, &pos))
		{
			SetCursorPos((int)pos.x, (int)pos.y);
		}
	}

	void set_mouse_pos(const point2_t& pos)
	{
		set_mouse_pos(pos.x, pos.y);
	}

	cursor_t* create_cursor(i32_t width, i32_t height, i32_t hot_x, i32_t hot_y, const void* data)
	{
		AUX_DEBUG_ASSERT(width > 0);
		AUX_DEBUG_ASSERT(height > 0);
		AUX_DEBUG_ASSERT((hot_x >= 0) && (hot_x < width));
		AUX_DEBUG_ASSERT((hot_y >= 0) && (hot_y < height));
		AUX_DEBUG_ASSERT(data != nullptr);

		if ((width > MAX_CURSOR_DIM) || (height > MAX_CURSOR_DIM))
		{
			return nullptr;
		}

		HBITMAP color_map = create_cursor_color_map(width, height, data);

		if (color_map != nullptr)
		{
			HBITMAP mask_map = create_cursor_mask_map(width, height);

			if (mask_map != nullptr)
			{
				HCURSOR handle = create_cursor_object(color_map, mask_map, hot_x, hot_y);

				if (handle != nullptr)
				{
					cursor_t* cursor = (cursor_t*)alloc_mem(sizeof(cursor_t));
					cursor->handle = handle;
					DeleteObject(mask_map);
					DeleteObject(color_map);
					return cursor;
				}

				DeleteObject(mask_map);
			}

			DeleteObject(color_map);
		}

		return nullptr;
	}

	cursor_t* create_cursor(const size2_t& size, const point2_t& hot_spot, const void* data)
	{
		return create_cursor(size.x, size.y, hot_spot.x, hot_spot.y, data);
	}

	void destroy_cursor(cursor_t* cursor)
	{
		if (cursor->handle == input->current_cursor)
		{
			set_current_cursor(nullptr);
		}

		DestroyCursor(cursor->handle);
		free_mem(cursor);
	}

	void set_cursor(cursor_t* cursor)
	{
		set_current_cursor(cursor->handle);
	}

	void set_default_cursor()
	{
		set_current_cursor(input->default_cursor);
	}
}
