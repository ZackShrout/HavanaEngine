#include "Platform.h"
#include "PlatformTypes.h"

namespace Havana::Platform
{
#ifdef _WIN64	// open window for DirectX context
	namespace
	{
		struct WindowInfo
		{
			HWND	hwnd{ nullptr };
			RECT	clientArea{ 0, 0, 1580, 950 };
			RECT	fullScreenArea{};
			POINT	topLeft{ 0,0 };
			DWORD	style{ WS_VISIBLE };
			bool	isFullscreen{ false };
			bool	isClosed{ false };
		};

		LRESULT CALLBACK internal_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
		{
			LONG_PTR longPtr{ GetWindowLongPtr(hwnd, 0) };
			return longPtr
				? ((window_proc)longPtr)(hwnd, msg, wparam, lparam)
				: DefWindowProc(hwnd, msg, wparam, lparam);
		}
	} // anonymous namespace

	Window MakeWindow(const WindowInitInfo* const initInfo /*= nullptr*/)
	{
		window_proc callback{ initInfo ? initInfo->callback : nullptr };
		window_handle parent{ initInfo ? initInfo->parent : nullptr };

		// Setup a window class
		WNDCLASSEX wc;
		ZeroMemory(&wc, sizeof(wc));
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = internal_window_proc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = callback ? sizeof(callback) : 0;
		wc.hInstance = 0;
		wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = CreateSolidBrush(RGB(26, 48, 76));
		wc.lpszMenuName = NULL;
		wc.lpszClassName = L"HavanaWindow";
		wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

		// Register the window class
		RegisterClassEx(&wc);

		// Create an instance of WindowInfo
		WindowInfo info{};
		RECT rc{ info.clientArea };
		
		// adjust rectangle to fit the window while accounting for border, header, etc..
		AdjustWindowRect(&rc, info.style, FALSE);

		// check for initial info, use defaults if none given
		const wchar_t* caption{ (initInfo && initInfo->caption) ? initInfo->caption : L"Havana Game" };
		const s32 left{ (initInfo && initInfo->left) ? initInfo->left : info.clientArea.left };
		const s32 top{ (initInfo && initInfo->top) ? initInfo->top : info.clientArea.top };
		const s32 width{ (initInfo && initInfo->width) ? initInfo->width : rc.right - rc.left};
		const s32 height{ (initInfo && initInfo->height) ? initInfo->height : rc.bottom - rc.top};
		
		info.style |= parent ? WS_CHILD : WS_OVERLAPPEDWINDOW;

		// Create instance of the window class
		info.hwnd = CreateWindowEx(
			0,					// extended style
			wc.lpszClassName,	// window class name
			caption,			// instance title
			info.style,			// window style
			left, top,			// initial window coords
			width, height,		// initial window dimensions
			parent,				// handle to parent window
			NULL,				// handle to menu
			NULL,				// instance of this application
			NULL);				// extra creation parameters

		if (info.hwnd)
		{
			
		}
		
		return Window();
	}

	void RemoveWindow(window_id id)
	{

	}
#elif define(__APPLE__)
	// OSX stuff here... open window for Metal context
#elif define(__linux__)
	// Linux stuff here... open window for OpenGL or Vulkan context
#elif
#error Must implement at least one platform.
#endif
}