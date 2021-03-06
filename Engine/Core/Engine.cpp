#ifndef SHIPPING

#include<thread>
#include "../Content/ContentLoader.h"
#include "../Components/Script.h"
#include "../Platforms/PlatformTypes.h"
#include "../Platforms/Platform.h" 
#include "../Graphics/Renderer.h"

using namespace Havana;

#ifdef _WIN64

namespace
{
	Graphics::RenderSurface gameWindow{};
	
	LRESULT WinProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
	{
		switch (msg)
		{
		case WM_DESTROY:
		{
			if (gameWindow.window.IsClosed())
			{
				PostQuitMessage(0);
				return 0;
			}
			break;
		}
		case WM_SYSCHAR:
			if (wparam == VK_RETURN && (HIWORD(lparam) & KF_ALTDOWN))
			{
				gameWindow.window.SetFullscreen(!gameWindow.window.IsFullscreen());
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
	if(!Content::LoadGame()) return false;

	Platform::WindowInitInfo info
	{
		&WinProc, nullptr, L"Havana Game" // TODO: get the game name from the loaded game file
	};

	gameWindow.window = Platform::MakeWindow(&info);

	if (!gameWindow.window.IsValid()) return false;

	return true;
}

void EngineUpdate()
{
	Havana::Script::Update(10.0f);
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

void EngineShutdown()
{
	Platform::RemoveWindow(gameWindow.window.GetID());
	Havana::Content::UnloadGame();
}

#endif // _Win64

#endif // !SHIPPING