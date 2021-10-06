#pragma once
#include "CommonHeaders.h"

#ifdef _WIN64

#ifndef WIN_32_LEAN_AND_MEAN
#define WIN_32_LEAN_AND_MEAN
#endif // WIN_32_LEAN_AND_MEAN

#include <Windows.h>

namespace Havana::Platform
{
	using window_proc = LRESULT(*)(HWND, UINT, WPARAM, LPARAM);
	using window_handle = HWND;

	struct WindowInitInfo
	{
		window_proc		callback{ nullptr };
		window_handle	parent{ nullptr };
		const wchar_t*	caption{ nullptr };
		s32				left{ 0 };
		s32				top{ 0 };
		s32				width{ 1580 };
		s32				height{ 950 };
	};
}

#endif // _WIN64

#ifdef __linux__

#include <GL/glx.h>
#include <X11/Xlib.h>
#include <stdlib.h>

// Prevents collision from our Window class and the XWindow Window define
using XWindow = Window;

namespace Havana::Platform
{
	using glXCreateContextAttribsARBProc = 
		GLXContext (*)(Display*, GLXFBConfig, GLXContext, Bool, const int*);
	
	using window_handle = XWindow*;

	struct WindowInitInfo
	{
		void*			callback{ nullptr };
		window_handle	parent{ nullptr };
		const wchar_t*	caption{ nullptr };
		s32				left{ 0 };
		s32				top{ 0 };
		s32				width{ 1580 };
		s32				height{ 950 };
	};
}

#endif // __linux__