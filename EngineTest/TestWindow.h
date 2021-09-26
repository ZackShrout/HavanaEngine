#pragma once
#include "Test.h"
#include "../Platforms/PlatformTypes.h"
#include "../Platforms/Platform.h"

using namespace Havana;

Platform::Window windows[4];

#ifdef _Win64

LRESULT WinProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
	case WM_DESTROY:
	{
		bool allClosed{ true };
		for (u32 i{ 0 }; i < _countof(windows); i++)
		{
			if (!windows[i].IsClosed())
			{
				allClosed = false;
			}
		}
		if (allClosed)
		{
			PostQuitMessage(0);
			return 0;
		}
		break;
	}
	case WM_SYSCHAR:
		if (wparam == VK_RETURN && (HIWORD(lparam) & KF_ALTDOWN))
		{
			Platform::Window win{ Platform::window_id{(Id::id_type)GetWindowLongPtr(hwnd, GWLP_USERDATA)} };
			win.SetFullscreen(!win.IsFullscreen());
			return 0;
		}
		break;
	default:
		break;
	}

	return DefWindowProc(hwnd, msg, wparam, lparam);
}

#elif __linux__

XEvent WinProc(Display* display)
{
	// TODO: this needs to be implemented		
	
	XEvent xev;

	return xev;
}

#endif // _WIN64

class EngineTest : public Test
{
public:
	bool Initialize() override
	{
		Platform::WindowInitInfo info[]
		{
			{ &WinProc, nullptr, L"Test Window 1", 100, 100, 400, 800 },
			{ &WinProc, nullptr, L"Test Window 2", 150, 150, 800, 400 },
			{ &WinProc, nullptr, L"Test Window 3", 200, 200, 400, 400 },
			{ &WinProc, nullptr, L"Test Window 4", 250, 250, 800, 600 },
		};

		static_assert(_countof(info) == _countof(windows));

		for (u32 i{ 0 }; i < _countof(windows); i++)
		{
			windows[i] = Platform::MakeWindow(&info[i]);
		}

		return true;
	}

	void Run() override
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(10000));
	}

	void Shutdown() override
	{
		for (u32 i{ 0 }; i < _countof(windows); i++)
		{
			Platform::RemoveWindow(windows[i].GetID());
		}
	}
private:

};