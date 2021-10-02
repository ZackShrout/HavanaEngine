#pragma once
#include "CommonHeaders.h"
#include "Window.h"
namespace Havana::Platform
{
	struct WindowInitInfo; // forward declaration, defined by platform in PlatformTypes.h
#ifdef _WIN64
	Window MakeWindow(const WindowInitInfo* const initInfo = nullptr);
#elif __linux__
	Window MakeWindow(const WindowInitInfo* const initInfo = nullptr, void* disp = nullptr);
#endif
	void RemoveWindow(window_id id);
}