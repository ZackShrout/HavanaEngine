#pragma once
#include "CommonHeaders.h"
#include "Window.h"
namespace Havana::Platform
{
	struct WindowInitInfo; // forward declaration, defined by platform in PlatformTypes.h
	Window MakeWindow(const WindowInitInfo* const initInfo = nullptr, void* disp = nullptr);
	void RemoveWindow(window_id id);
}