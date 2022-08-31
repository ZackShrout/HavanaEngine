#pragma once
#ifdef _WIN64

#include "Test.h"
#include "Platforms/PlatformTypes.h"
#include "Platforms/Platform.h"

using namespace havana;

platform::window windows[4];

LRESULT win_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
	case WM_DESTROY:
	{
		bool all_closed{ true };
		for (u32 i{ 0 }; i < _countof(windows); i++)
		{
			if (!windows[i].is_closed())
			{
				all_closed = false;
			}
		}
		if (all_closed)
		{
			PostQuitMessage(0);
			return 0;
		}
		break;
	}
	case WM_SYSCHAR:
		if (wparam == VK_RETURN && (HIWORD(lparam) & KF_ALTDOWN))
		{
			platform::window win{ platform::window_id{(id::id_type)GetWindowLongPtr(hwnd, GWLP_USERDATA)} };
			win.set_fullscreen(!win.is_fullscreen());
			return 0;
		}
		break;
	default:
		break;
	}

	return DefWindowProc(hwnd, msg, wparam, lparam);
}

class engine_test : public test
{
public:
	bool initialize() override
	{
		platform::window_init_info info[]
		{
			{ &win_proc, nullptr, L"Test Window 1", 100, 100, 400, 800 },
			{ &win_proc, nullptr, L"Test Window 2", 150, 150, 800, 400 },
			{ &win_proc, nullptr, L"Test Window 3", 200, 200, 400, 400 },
			{ &win_proc, nullptr, L"Test Window 4", 250, 250, 800, 600 },
		};

		static_assert(_countof(info) == _countof(windows));

		for (u32 i{ 0 }; i < _countof(windows); ++i)
		{
			windows[i] = platform::create_window(&info[i]);
		}

		return true;
	}

	void run() override
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	void shutdown() override
	{
		for (u32 i{ 0 }; i < _countof(windows); i++)
		{
			platform::remove_window(windows[i].get_id());
		}
	}

};

#endif // WIN64