#pragma once
#include "../Common/CommonHeaders.h"
#include "Window.h"
namespace Havana::Platform
{
	struct WindowInitInfo; // forward declaration, defined by platform in PlatformTypes.h

	Window MakeWindow(const WindowInitInfo* const initInfo = nullptr);
	void RemoveWindow(window_id id);
}