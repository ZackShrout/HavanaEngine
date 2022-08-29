#ifndef SHIPPING

#include<thread>
#include "../Content/ContentLoader.h"
#include "../Components/Script.h"
#include "../Platforms/PlatformTypes.h"
#include "../Platforms/Platform.h" 
#include "../Graphics/Renderer.h"

using namespace havana;

#ifdef _WIN64

namespace
{
	graphics::RenderSurface gameWindow{};
	
	LRESULT WinProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
	{
		switch (msg)
		{
		case WM_DESTROY:
		{
			if (gameWindow.window.is_closed())
			{
				PostQuitMessage(0);
				return 0;
			}
			break;
		}
		case WM_SYSCHAR:
			if (wparam == VK_RETURN && (HIWORD(lparam) & KF_ALTDOWN))
			{
				gameWindow.window.set_fullscreen(!gameWindow.window.is_fullscreen());
				return 0;
			}
			break;
		default:
			break;
		}

		return DefWindowProc(hwnd, msg, wparam, lparam);
	}
} // anonymous namespace

bool EngineInitialize()
{
	if(!content::load_game()) return false;

	platform::window_init_info info
	{
		&WinProc, nullptr, L"Havana Game" // TODO: get the game name from the loaded game file
	};

	gameWindow.window = platform::create_window(&info);

	if (!gameWindow.window.is_valid()) return false;

	return true;
}

void EngineUpdate()
{
	havana::script::update(10.0f);
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

void EngineShutdown()
{
	platform::remove_window(gameWindow.window.get_id());
	havana::content::unload_game();
}

#endif // _Win64

#endif // !SHIPPING