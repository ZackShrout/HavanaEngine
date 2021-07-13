#include "TestRenderer.h"
#if TEST_RENDERER
#include "..\Platforms\PlatformTypes.h"
#include "..\Platforms\Platform.h"
#include "..\Graphics\Renderer.h"


using namespace Havana;

Graphics::RenderSurface surfaces[4];

LRESULT WinProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
	case WM_DESTROY:
	{
		bool allClosed{ true };
		for (u32 i{ 0 }; i < _countof(surfaces); i++)
		{
			if (!surfaces[i].window.IsClosed())
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

void CreateRenderSurface(Graphics::RenderSurface& surface, Platform::WindowInitInfo info)
{
	surface.window = Platform::MakeWindow(&info);
}

void DestroyRenderSurface(Graphics::RenderSurface& surface)
{
	Platform::RemoveWindow(surface.window.GetID());
}

bool EngineTest::Initialize()
{
	bool result{ Graphics::Initialize(Graphics::GraphicsPlatform::Direct3D12) };

	if (!result) return result;

	Platform::WindowInitInfo info[]
	{
		{ &WinProc, nullptr, L"Test Window 1", 100, 100, 400, 800 },
		{ &WinProc, nullptr, L"Test Window 2", 150, 150, 800, 400 },
		{ &WinProc, nullptr, L"Test Window 3", 200, 200, 400, 400 },
		{ &WinProc, nullptr, L"Test Window 4", 250, 250, 800, 600 },
	};

	static_assert(_countof(info) == _countof(surfaces));

	for (u32 i{ 0 }; i < _countof(surfaces); i++)
		CreateRenderSurface(surfaces[i], info[i]);

	return result;
}

void EngineTest::Run()
{
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
	Graphics::Render();
}

void EngineTest::Shutdown()
{
	for (u32 i{ 0 }; i < _countof(surfaces); i++)
		DestroyRenderSurface(surfaces[i]);

	Graphics::Shutdown();
}
#endif //TEST_RENDERER