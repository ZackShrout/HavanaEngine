#ifdef _WIN64

#include "Platform.h"
#include "PlatformTypes.h"

namespace havana::platform
{
	namespace
	{
		// Windows OS specific window info
		struct WindowInfo
		{
			HWND hwnd{nullptr};
			HINSTANCE hinstance{ nullptr };
			RECT clientArea{0, 0, 1580, 950};
			RECT fullScreenArea{};
			POINT topLeft{0, 0};
			DWORD style{WS_VISIBLE};
			bool isFullscreen{false};
			bool isClosed{false};
		};

		utl::free_list<WindowInfo> windows;

		WindowInfo &get_from_id(window_id id)
		{
			assert(windows[id].hwnd);
			return windows[id];
		}

		WindowInfo &get_from_handle(window_handle handle)
		{
			const window_id id{(id::id_type)GetWindowLongPtr(handle, GWLP_USERDATA)};
			return get_from_id(id);
		}

		HINSTANCE get_display(window_id id)
		{
			return get_from_id(id).hinstance;
		}

		bool resized{ false };

		// Callback method for message handling
		LRESULT CALLBACK internal_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
		{
			switch (msg)
			{
			case WM_NCCREATE:
			{
				// Put the window id in the user data field of window's data buffer.
				DEBUG_OP(SetLastError(0));
				const window_id id{ windows.add() };
				windows[id].hwnd = hwnd;
				// Include the window ID in the window class data structure
				SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)id);
				assert(GetLastError() == 0);
			}
			break;
			case WM_DESTROY:
				get_from_handle(hwnd).isClosed = true;
				break;
			case WM_SIZE:
				resized = (wparam != SIZE_MINIMIZED);
				break;
			default:
				break;
			}

			if (resized && GetAsyncKeyState(VK_LBUTTON) >= 0)
			{
				WindowInfo& info{ get_from_handle(hwnd) };
				assert(info.hwnd);
				GetClientRect(info.hwnd, info.isFullscreen ? &info.fullScreenArea : &info.clientArea);
				resized = false;
			}

			// "Extra" bytes for handling windows messages via callback
			LONG_PTR longPtr{GetWindowLongPtr(hwnd, 0)};
			return longPtr
					   ? ((window_proc)longPtr)(hwnd, msg, wparam, lparam)
					   : DefWindowProc(hwnd, msg, wparam, lparam);
		}

		// Windows specific Window class function implementations
		void resize_window(const WindowInfo &info, const RECT &area)
		{
			// Adjust the window size for correct device size
			RECT windowRect{area};
			AdjustWindowRect(&windowRect, info.style, FALSE);

			const s32 width{windowRect.right - windowRect.left};
			const s32 height{windowRect.bottom - windowRect.top};

			MoveWindow(info.hwnd, info.topLeft.x, info.topLeft.y, width, height, true);
		}

		void resize_window(window_id id, u32 width, u32 height)
		{
			WindowInfo &info{get_from_id(id)};

			// NOTE: when we host a window in the level editor we just update
			// the internal data (i.e. the client area dimensions).
			if (info.style & WS_CHILD)
			{
				GetClientRect(info.hwnd, &info.clientArea);
			}
			else
			{
				// NOTE: resize in fullscreen mode as well to support the case when
				// the user changes screen resolution
				RECT &area{info.isFullscreen ? info.fullScreenArea : info.clientArea};
				area.bottom = area.top + height;
				area.right = area.left + width;

				resize_window(info, area);
			}
		}

		void set_window_fullscreen(window_id id, bool isFullscreen)
		{
			WindowInfo &info{get_from_id(id)};
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
					SetWindowLongPtr(info.hwnd, GWL_STYLE, 0);
					ShowWindow(info.hwnd, SW_MAXIMIZE);
				}
				else
				{
					SetWindowLongPtr(info.hwnd, GWL_STYLE, info.style);
					resize_window(info, info.clientArea);
					ShowWindow(info.hwnd, SW_SHOWNORMAL);
				}
			}
		}

		bool is_window_fullscreen(window_id id)
		{
			return get_from_id(id).isFullscreen;
		}

		window_handle get_window_handle(window_id id)
		{
			return get_from_id(id).hwnd;
		}

		void set_window_caption(window_id id, const wchar_t *caption)
		{
			WindowInfo &info{get_from_id(id)};
			SetWindowText(info.hwnd, caption);
		}

		math::u32v4 get_window_size(window_id id)
		{
			WindowInfo &info{get_from_id(id)};
			RECT &area{info.isFullscreen ? info.fullScreenArea : info.clientArea};
			return {(u32)area.left, (u32)area.top, (u32)area.right, (u32)area.bottom};
		}

		bool is_window_closed(window_id id)
		{
			return get_from_id(id).isClosed;
		}

		void set_window_closed(window_id id)
		{
			get_from_id(id).isClosed = true;
		}
	} // anonymous namespace

	/// <summary>
	/// Create a new window.
	/// </summary>
	/// <param name="initInfo"> - Initialization information for the new window.</param>
	/// <returns>A havana::Platform::Window object.</returns>
	window create_window(const window_init_info* initInfo /*= nullptr*/, void* disp /*= nullptr*/) // NOTEL CreateWindow collides with Windows.h
	{
		window_proc callback{initInfo ? initInfo->callback : nullptr};
		window_handle parent{initInfo ? initInfo->parent : nullptr};

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
		info.clientArea.right = (initInfo && initInfo->width) ? info.clientArea.left + initInfo->width : info.clientArea.right;
		info.clientArea.bottom = (initInfo && initInfo->height) ? info.clientArea.top + initInfo->height : info.clientArea.bottom;
		info.style |= parent ? WS_CHILD : WS_OVERLAPPEDWINDOW;
		info.hinstance = wc.hInstance;
		RECT rect{info.clientArea};

		// Adjust the window size for correct device size
		AdjustWindowRect(&rect, info.style, FALSE);

		// check for initial info, use defaults if none given
		const wchar_t *caption{(initInfo && initInfo->caption) ? initInfo->caption : L"Havana Game"};
		const s32 left{initInfo ? initInfo->left : info.topLeft.x};
		const s32 top{initInfo ? initInfo->top : info.topLeft.y};
		const s32 width{rect.right - rect.left};
		const s32 height{rect.bottom - rect.top};

		// Create instance of the window class
		info.hwnd = CreateWindowEx(
			0,				  // extended style
			wc.lpszClassName, // window class name
			caption,		  // instance title
			info.style,		  // window style
			left, top,		  // initial window coords
			width, height,	  // initial window dimensions
			parent,			  // handle to parent window
			NULL,			  // handle to menu
			NULL,			  // instance of this application
			NULL);			  // extra creation parameters

		if (info.hwnd)
		{
			// Set  the pointer to the window callback function which handles messages
			// for the windows in the "extra" bytes
			DEBUG_OP(SetLastError(0));
			if (callback) SetWindowLongPtr(info.hwnd, 0, (LONG_PTR)callback);
			assert(GetLastError() == 0);
			ShowWindow(info.hwnd, SW_SHOWNORMAL);
			UpdateWindow(info.hwnd);

			window_id id{ (id::id_type)GetWindowLongPtr(info.hwnd, GWLP_USERDATA) };
			windows[id] = info;

			return window{id};
		}

		return {};
	}

	/// <summary>
	/// Remove an existing window.
	/// </summary>
	/// <param name="id"> - ID number of the window to remove.</param>
	void remove_window(window_id id)
	{
		WindowInfo &info{get_from_id(id)};
		DestroyWindow(info.hwnd);
		windows.remove(id);
	}
}

#include "IncludeWindowCpp.h"

#endif // _WIN64