#pragma once
#include "CommonHeaders.h"
#include "Window.h"
namespace havana::platform
{
	struct window_init_info; // forward declaration, defined by platform in PlatformTypes.h
	window create_window(const window_init_info* const initInfo = nullptr, void* disp = nullptr);
	void remove_window(window_id id);
}