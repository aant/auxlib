#pragma once

#include "point2.h"
#include "size2.h"

namespace aux
{
	enum
	{
		KEY_BAD_ENUM = -1,

		// Mouse buttons
		KEY_MOUSE_LEFT,
		KEY_MOUSE_RIGHT,
		KEY_MOUSE_MIDDLE,
		KEY_MOUSE_X1,
		KEY_MOUSE_X2,

		// System keys
		KEY_ESCAPE,
		KEY_ALT_LEFT,
		KEY_ALT_RIGHT,
		KEY_SHIFT_LEFT,
		KEY_SHIFT_RIGHT,
		KEY_CONTROL_LEFT,
		KEY_CONTROL_RIGHT,
		KEY_COMMAND_LEFT,
		KEY_COMMAND_RIGHT,
		KEY_CAPS_LOCK,
		KEY_SCROLL_LOCK,
		KEY_PAUSE,
		KEY_BREAK,
		KEY_SNAPSHOT,
		KEY_APPLICATION,
		KEY_SYSRQ,

		// Navigation keys
		KEY_ARROW_LEFT,
		KEY_ARROW_RIGHT,
		KEY_ARROW_UP,
		KEY_ARROW_DOWN,
		KEY_HOME,
		KEY_END,
		KEY_PAGE_UP,
		KEY_PAGE_DOWN,

		// Editing keys
		KEY_INSERT,
		KEY_DELETE,
		KEY_BACKSPACE,
		KEY_ENTER,

		// Punctuation keys
		KEY_SPACE,
		KEY_TAB,
		KEY_COMMA,
		KEY_PERIOD,
		KEY_APOSTROPHE,
		KEY_BACKTICK,
		KEY_SEMICOLON,
		KEY_SLASH,
		KEY_BACKSLASH,
		KEY_BRACKET_LEFT,
		KEY_BRACKET_RIGHT,
		KEY_MINUS,
		KEY_EQUALS,

		// Alphabetic keys
		KEY_A,
		KEY_B,
		KEY_C,
		KEY_D,
		KEY_E,
		KEY_F,
		KEY_G,
		KEY_H,
		KEY_I,
		KEY_J,
		KEY_K,
		KEY_L,
		KEY_M,
		KEY_N,
		KEY_O,
		KEY_P,
		KEY_Q,
		KEY_R,
		KEY_S,
		KEY_T,
		KEY_U,
		KEY_V,
		KEY_W,
		KEY_X,
		KEY_Y,
		KEY_Z,

		// Digital keys
		KEY_0,
		KEY_1,
		KEY_2,
		KEY_3,
		KEY_4,
		KEY_5,
		KEY_6,
		KEY_7,
		KEY_8,
		KEY_9,

		// Functional keys
		KEY_F1,
		KEY_F2,
		KEY_F3,
		KEY_F4,
		KEY_F5,
		KEY_F6,
		KEY_F7,
		KEY_F8,
		KEY_F9,
		KEY_F10,
		KEY_F11,
		KEY_F12,

		// Numeric pad keys
		KEY_NUM_LOCK,
		KEY_NUM0,
		KEY_NUM1,
		KEY_NUM2,
		KEY_NUM3,
		KEY_NUM4,
		KEY_NUM5,
		KEY_NUM6,
		KEY_NUM7,
		KEY_NUM8,
		KEY_NUM9,
		KEY_NUM_PLUS,
		KEY_NUM_MINUS,
		KEY_NUM_MULTIPLY,
		KEY_NUM_DIVIDE,
		KEY_NUM_CLEAR,
		KEY_NUM_DELETE,
		KEY_NUM_DECIMAL,
		KEY_NUM_INSERT,
		KEY_NUM_ENTER,

		// Other keys
		KEY_OEM102,

		KEY_MAX_ENUMS
	};

	struct cursor_t;

	struct input_caps_t
	{
		i32_t max_cursor_dim;
	};

	struct input_handler_t
	{
		void* user_ptr;
		void(*on_key_down)(void* user_ptr, e32_t key);
		void(*on_key_up)(void* user_ptr, e32_t key);
		void(*on_double_click)(void* user_ptr, e32_t key);
		void(*on_mouse_move)(void* user_ptr, const point2_t& pos);
		void(*on_mouse_scroll)(void* user_ptr, i32_t pos);
	};

	const input_caps_t& get_input_caps();

	bool is_key_down(e32_t key);
	bool is_key_on(e32_t key);

	const point2_t& get_mouse_pos();
	void set_mouse_pos(i32_t x, i32_t y);
	void set_mouse_pos(const point2_t& pos);

	cursor_t* create_cursor(i32_t width, i32_t height, i32_t hot_x, i32_t hot_y, const void* data);
	cursor_t* create_cursor(const size2_t& size, const point2_t& hot_spot, const void* data);
	void destroy_cursor(cursor_t* cursor);
	void set_cursor(cursor_t* cursor);
	void set_default_cursor();

	extern input_handler_t input_handler;
}
