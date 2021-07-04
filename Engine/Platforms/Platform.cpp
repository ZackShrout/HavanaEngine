#include "Platform.h"
#include "PlatformTypes.h"

namespace Havana::Platform
{
#ifdef _WIN64	// open window for DirectX context
	namespace
	{
		// Windows OS specific window info
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

		Utils::vector<WindowInfo> windows;

		/////////////////////////////////////////////////////////////////
		// TODO: this part will be handled by a free-list container later
		Utils::vector<u32> availableSlots;

		u32 AddToWindows(WindowInfo info)
		{
			u32 id{ U32_INVALID_ID };
			if (availableSlots.empty())
			{
				id = (u32)windows.size();
				windows.emplace_back(info);
			}
			else
			{
				id = availableSlots.back();
				availableSlots.pop_back();
				assert(id != U32_INVALID_ID);
				windows[id] = info;
			}

			return id;
		}

		void RemoveFromWindows(u32 id)
		{
			assert(id < windows.size());
			availableSlots.emplace_back(id);
		}
		/////////////////////////////////////////////////////////////////

		WindowInfo& GetFromId(window_id id)
		{
			assert(id < windows.size());
			assert(windows[id].hwnd);
			return windows[id];
		}

		WindowInfo& GetFromHandle(window_handle handle)
		{
			const window_id id{ (Id::id_type)GetWindowLongPtr(handle, GWLP_USERDATA) };
			return GetFromId(id);
		}
		
		// "Extra" bytes for handling windows messages via callback
		LRESULT CALLBACK internal_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
		{
			WindowInfo* info{ nullptr };
			switch (msg)
			{
			case WM_DESTROY:
				GetFromHandle(hwnd).isClosed = true;
				break;

			}
			
			LONG_PTR longPtr{ GetWindowLongPtr(hwnd, 0) };
			return longPtr
				? ((window_proc)longPtr)(hwnd, msg, wparam, lparam)
				: DefWindowProc(hwnd, msg, wparam, lparam);
		}

		// Windows specific Window class function implementations
		void ResizeWindow(const WindowInfo& info, const RECT& area)
		{
			// Adjust the window size for correct device size
			RECT windowRect{ area };
			AdjustWindowRect(&windowRect, info.style, FALSE);

			const s32 width{ windowRect.right - windowRect.left };
			const s32 height{ windowRect.bottom - windowRect.top };

			MoveWindow(info.hwnd, info.topLeft.x, info.topLeft.y, width, height, true);
		}

		void ResizeWindow(window_id id, u32 width, u32 height)
		{
			WindowInfo& info{ GetFromId(id) };
			// NOTE: resize in fullscreen mode as well to support the case when 
			// the user changes screen resolution
			RECT& area{ info.isFullscreen ? info.fullScreenArea : info.clientArea };
			area.bottom = area.top + height;
			area.right = area.left + width;

			ResizeWindow(info, area);
		}

		void SetWindowFullscreen(window_id id, bool isFullscreen)
		{
			WindowInfo& info{ GetFromId(id) };
			if (info.isFullscreen != isFullscreen)
			{
				info.isFullscreen = isFullscreen;

				if (isFullscreen)
				{
					// Store the current window position and dimenstions so it can be restored
					// when switching out of a full screen state.
					GetClientRect(info.hwnd, &info.clientArea);
					RECT rect;
					GetWindowRect(info.hwnd, &rect);
					info.topLeft.x = rect.left;
					info.topLeft.y = rect.top;
					info.style = 0;
					SetWindowLongPtr(info.hwnd, GWL_STYLE, info.style);
					ShowWindow(info.hwnd, SW_MAXIMIZE);
				}
				else
				{
					info.style = WS_VISIBLE | WS_OVERLAPPEDWINDOW;
					SetWindowLongPtr(info.hwnd, GWL_STYLE, info.style);
					ResizeWindow(info, info.clientArea);
					ShowWindow(info.hwnd, SW_SHOWNORMAL);
				}
			}
		}

		bool IsWindowFullscreen(window_id id)
		{
			return GetFromId(id).isFullscreen;
		}

		window_handle GetWindowHandle(window_id id)
		{
			return GetFromId(id).hwnd;
		}

		void SetWindowCaption(window_id id, const wchar_t* caption)
		{
			WindowInfo& info{ GetFromId(id) };
			SetWindowText(info.hwnd, caption);
		}

		Math::Vec4u32 GetWindowSize(window_id id)
		{
			WindowInfo& info{ GetFromId(id) };
			RECT area{ info.isFullscreen ? info.fullScreenArea : info.clientArea };
			return { (u32)area.left, (u32)area.top, (u32)area.right, (u32)area.bottom };
		}

		bool IsWindowClosed(window_id id)
		{
			return GetFromId(id).isClosed;
		}
	} // anonymous namespace

	/// <summary>
	/// Create a new window.
	/// </summary>
	/// <param name="initInfo"> - Initialization information for the new window.</param>
	/// <returns>A Havana::Platform::Window object.</returns>
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
		
		// Adjust the window size for correct device size
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
			const window_id id{ AddToWindows(info) };
			// Include the window ID in the window class data structure
			SetWindowLongPtr(info.hwnd, GWLP_USERDATA, (LONG_PTR)id);

			// Set  the pointer to the window callback function which handles messages
			// for the windows in the "extra" bytes
			if (callback) SetWindowLongPtr(info.hwnd, 0, (LONG_PTR)callback);
			assert(GetLastError() == 0);

			ShowWindow(info.hwnd, SW_SHOWNORMAL);
			UpdateWindow(info.hwnd);
			return Window{ id };
		}
		
		return {};
	}

	/// <summary>
	/// Remove an existing window.
	/// </summary>
	/// <param name="id"> - ID number of the window to remove.</param>
	void RemoveWindow(window_id id)
	{
		WindowInfo& info{ GetFromId(id) };
		DestroyWindow(info.hwnd);
		RemoveFromWindows(id);
	}
#elif define(__APPLE__)
	// OSX stuff here... open window for Metal context
#elif define(__linux__)
	// Linux stuff here... open window for OpenGL or Vulkan context
#elif
#error Must implement at least one platform.
#endif

	//***************************************************************
	// Implementation of the Window class abstraction layer functions
	//***************************************************************
	
	void Window::SetFullscreen(bool isFullscreen) const
	{
		assert(IsValid());
		SetWindowFullscreen(id, isFullscreen);
	}

	bool Window::IsFullscreen() const
	{
		assert(IsValid());
		return IsWindowFullscreen(id);
	}

	void* Window::Handle() const
	{
		assert(IsValid());
		return GetWindowHandle(id);
	}

	void Window::SetCaption(const wchar_t* caption) const
	{
		assert(IsValid());
		SetWindowCaption(id, caption);
	}

	const Math::Vec4u32 Window::Size() const
	{
		assert(IsValid());
		return GetWindowSize(id);
	}

	void Window::Resize(u32 width, u32 height) const
	{
		assert(IsValid());
		ResizeWindow(id, width, height);
	}

	const u32 Window::Width() const
	{
		Math::Vec4u32 size{ Size() };
		return size.z - size.x;
	}

	const u32 Window::Height() const
	{
		Math::Vec4u32 size{ Size() };
		return size.w - size.y;
	}

	bool Window::IsClosed() const
	{
		assert(IsValid());
		return IsWindowClosed(id);
	}

}