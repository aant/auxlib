#pragma once

#include "point2.h"
#include "size2.h"

namespace aux
{
	enum
	{
		Key_BadEnum = -1,

		// Mouse Buttons
		Key_MouseLeft,
		Key_MouseRight,
		Key_MouseMiddle,
		Key_MouseX1,
		Key_MouseX2,

		// System Keys
		Key_Escape,
		Key_AltLeft,
		Key_AltRight,
		Key_ShiftLeft,
		Key_ShiftRight,
		Key_ControlLeft,
		Key_ControlRight,
		Key_CommandLeft,
		Key_CommandRight,
		Key_CapsLock,
		Key_ScrollLock,
		Key_Pause,
		Key_Break,
		Key_Snapshot,
		Key_Application,
		Key_SysReq,

		// Navigation Keys
		Key_ArrowLeft,
		Key_ArrowRight,
		Key_ArrowUp,
		Key_ArrowDown,
		Key_Home,
		Key_End,
		Key_PageUp,
		Key_PageDown,

		// Editing Keys
		Key_Insert,
		Key_Delete,
		Key_Backspace,
		Key_Enter,

		// Punctuation Keys
		Key_Space,
		Key_Tab,
		Key_Comma,
		Key_Period,
		Key_Apostrophe,
		Key_Backtick,
		Key_Semicolon,
		Key_Slash,
		Key_Backslash,
		Key_BracketLeft,
		Key_BracketRight,
		Key_Minus,
		Key_Equals,

		// Alphabetic Keys
		Key_A,
		Key_B,
		Key_C,
		Key_D,
		Key_E,
		Key_F,
		Key_G,
		Key_H,
		Key_I,
		Key_J,
		Key_K,
		Key_L,
		Key_M,
		Key_N,
		Key_O,
		Key_P,
		Key_Q,
		Key_R,
		Key_S,
		Key_T,
		Key_U,
		Key_V,
		Key_W,
		Key_X,
		Key_Y,
		Key_Z,

		// Digital Keys
		Key_0,
		Key_1,
		Key_2,
		Key_3,
		Key_4,
		Key_5,
		Key_6,
		Key_7,
		Key_8,
		Key_9,

		// Functional Keys
		Key_F1,
		Key_F2,
		Key_F3,
		Key_F4,
		Key_F5,
		Key_F6,
		Key_F7,
		Key_F8,
		Key_F9,
		Key_F10,
		Key_F11,
		Key_F12,

		// Numeric Pad Keys
		Key_NumLock,
		Key_Num0,
		Key_Num1,
		Key_Num2,
		Key_Num3,
		Key_Num4,
		Key_Num5,
		Key_Num6,
		Key_Num7,
		Key_Num8,
		Key_Num9,
		Key_NumPlus,
		Key_NumMinus,
		Key_NumMultiply,
		Key_NumDivide,
		Key_NumClear,
		Key_NumDelete,
		Key_NumDecimal,
		Key_NumInsert,
		Key_NumEnter,

		// Other Keys
		Key_Oem102,

		Key_MaxEnums
	};

	typedef struct CursorImpl* CursorHandle;

	struct InputCaps
	{
		int32 maxCursorDimension;
	};

	struct InputHandler
	{
		void* userPtr;
		void(*OnKeyDown)(void* userPtr, enum32 key);
		void(*OnKeyUp)(void* userPtr, enum32 key);
		void(*OnDoubleClick)(void* userPtr, enum32 key);
		void(*OnMouseMove)(void* userPtr, const Point2& position);
		void(*OnMouseScroll)(void* userPtr, int32 position);
	};

	const InputCaps& Input_GetCaps();

	bool Input_IsKeyDown(enum32 key);
	bool Input_IsKeyOn(enum32 key);

	const Point2& Input_GetMousePosition();
	void Input_PlaceMouse(const Point2& position);

	CursorHandle Input_CreateCursor(const Size2& size, const Point2& hotSpot, const void* data);
	void Input_DestroyCursor(CursorHandle cursor);
	void Input_SetCursor(CursorHandle cursor);
	void Input_SetDefaultCursor();

	extern InputHandler inputHandler;
}
