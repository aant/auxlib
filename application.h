#pragma once

#include "size2.h"

namespace aux
{
	enum
	{
		AppStyle_BadEnum = -1,

		AppStyle_Fullscreen,
		AppStyle_FixedWindow,
		AppStyle_ResizableWindow,

		AppStyle_MaxEnums,
	};

	struct ApplicationHandler
	{
		void* userPtr;
		bool(*OnInit)(void* userPtr);
		void(*OnFree)(void* userPtr);
		void(*OnSleep)(void* userPtr);
		void(*OnAwake)(void* userPtr);
		void(*OnUpdate)(void* userPtr, float32 timeDelta);
		void(*OnResize)(void* userPtr, const Size2& resolution);
		void(*OnRedraw)(void* userPtr);
	};

	enum32 Application_GetStyle();
	const Size2& Application_GetResolution();

	void Application_Start(const char title[], enum32 style, const Size2& resolution);
	void Application_Close();
	void Application_Reshape(enum32 style, const Size2& resolution);

	extern ApplicationHandler applicationHandler;
}
