#include "Platform.h"
#include "PlatformTypes.h"

namespace Havana::Platform
{
	#ifdef __linux__
		namespace
		{
			// Linux OS specific window info
			struct WindowInfo
			{
				XWindow window{};
				Display* display{ nullptr };
				s32 left;
				s32 top;
				s32 width;
				s32 height;
				bool isFullscreen{ false };
				bool isClosed{ false };
			};

			Utils::free_list<WindowInfo> windows;

			WindowInfo& GetFromId(window_id id)
			{
				assert(windows[id].window);
				return windows[id];
			}

			// Linux specific window class functions
			void ResizeWindow(window_id id, u32 width, u32 height)
			{
				WindowInfo& info{ GetFromId(id) };
				info.width = width;
				info.height = height;
				// NOTE: this is not currently working how I would like... I expected the window to redraw
				//		 itself with the XClearWindow() call. Eventually, the graphics API will be drawing
				//		 to the window anyway, so I will not pursue it further, unless it becomes an issue.
				XClearWindow(info.display, info.window);
			}

			void SetWindowFullscreen(window_id id, bool isFullscreen)
			{
				WindowInfo& info{ GetFromId(id) };
				if (info.isFullscreen != isFullscreen)
				{
					info.isFullscreen = isFullscreen;

					if (isFullscreen)
					{
						XEvent xev;
						Atom wm_state{ XInternAtom(info.display, "_NET_WM_STATE", false) };
						Atom fullscreen{ XInternAtom(info.display, "_NET_WM_STATE_FULLSCREEN", false) };
						memset(&xev, 0, sizeof(xev));
						xev.type = ClientMessage;
						xev.xclient.window = info.window;
						xev.xclient.message_type = wm_state;
						xev.xclient.format = 32;
						xev.xclient.data.l[0] = 1; // _NET_WM_STATE_ADD
						xev.xclient.data.l[1] = fullscreen;
						xev.xclient.data.l[2] = 0;
						XSendEvent(info.display, DefaultRootWindow(info.display), false,
							SubstructureNotifyMask | SubstructureRedirectMask, &xev);
					}
					else
					{
						XEvent xev;
						Atom wm_state{ XInternAtom(info.display, "_NET_WM_STATE", false) };
						Atom fullscreen{ XInternAtom(info.display, "_NET_WM_STATE_FULLSCREEN", false) };
						memset(&xev, 0, sizeof(xev));
						xev.type = ClientMessage;
						xev.xclient.window = info.window;
						xev.xclient.message_type = wm_state;
						xev.xclient.format = 32;
						xev.xclient.data.l[0] = 0; // _NET_WM_STATE_REMOVE
						xev.xclient.data.l[1] = fullscreen;
						xev.xclient.data.l[2] = 0;
						XSendEvent(info.display, DefaultRootWindow(info.display), false,
							SubstructureNotifyMask | SubstructureRedirectMask, &xev);
					}
				}
			}

			bool IsWindowFullscreen(window_id id)
			{
				return GetFromId(id).isFullscreen;
			}

			window_handle GetWindowHandle(window_id id)
			{
				return &GetFromId(id).window;
			}

			Display* GetDisplay(window_id id)
			{
				return GetFromId(id).display;
			}

			void SetWindowCaption(window_id id, const wchar_t* caption)
			{
				WindowInfo& info{ GetFromId(id) };
				size_t outSize = (sizeof(caption) * sizeof(wchar_t)) + 1;
				char title[outSize];
				wcstombs(title, caption, outSize);
				XStoreName(info.display, info.window, title);
			}

			Math::Vec4u32 GetWindowSize(window_id id)
			{
				WindowInfo& info{ GetFromId(id) };
				return { (u32)info.left, (u32)info.top, (u32)info.width - (u32)info.left, (u32)info.height - (u32)info.top };
			}

			bool IsWindowClosed(window_id id)
			{
				return GetFromId(id).isClosed;
			}

			void SetWindowClosed(window_id id)
			{
				WindowInfo& info{ GetFromId(id) };
				GetFromId(id).isClosed = true;
				XDestroyWindow(info.display, info.window);
			}
		} // anonymous namespace

		Window MakeWindow(const WindowInitInfo* const initInfo /*= nullptr*/, void* disp /*= nullptr*/)
		{
			// Cache a casted pointer of the display to save on casting later
			Display* display{ (Display*)disp };

			window_handle parent{ initInfo ? initInfo->parent : &(DefaultRootWindow(display)) };
			if (parent == nullptr)
			{
				parent = &(DefaultRootWindow(display));
			}
			assert(parent != nullptr);

			// Setup the screen, visual, and colormap
			int screen{ DefaultScreen(display) };
			Visual* visual{ DefaultVisual(display, screen) };
			Colormap colormap{ XCreateColormap(display, DefaultRootWindow(display),
											  visual, AllocNone) };

			// Define attributes for the window
			XSetWindowAttributes attributes;
			attributes.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask | StructureNotifyMask |
				ButtonPressMask | ButtonReleaseMask | PointerMotionMask;
			attributes.colormap = colormap;

			// Create an instance of WindowInfo
			WindowInfo info{};
			info.left = (initInfo && initInfo->left) ? initInfo->left : 0; // generally, the X window manager overrides
			info.top = (initInfo && initInfo->top) ? initInfo->top : 0;	   // the starting top left coords, so default is 0,0
			info.width = (initInfo && initInfo->width) ? initInfo->width : DisplayWidth(display, DefaultScreen(display));
			info.height = (initInfo && initInfo->height) ? initInfo->height : DisplayHeight(display, DefaultScreen(display));
			info.display = display;

			// check for initial info, use defaults if none given
			const wchar_t* caption{ (initInfo && initInfo->caption) ? initInfo->caption : L"Havana Game" };
			size_t outSize = (sizeof(caption) * sizeof(wchar_t)) + 1;
			char title[outSize];
			wcstombs(title, caption, outSize);

			XWindow window{ XCreateWindow(display, *parent, info.left, info.top, info.width, info.height, 0,
										 DefaultDepth(display, screen), InputOutput, visual,
										 CWColormap | CWEventMask, &attributes) };
			info.window = window;

			// Set custom window manager event for closing a window
			Atom wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", false);
			XSetWMProtocols(display, window, &wm_delete_window, 1);

			// Show window
			XMapWindow(display, window);
			XStoreName(display, window, title);

			const window_id id{ windows.add(info) };
			return Window{ id };
		}

		void RemoveWindow(window_id id)
		{
			WindowInfo& info{ GetFromId(id) };
			GetFromId(id).isClosed = true;
			XDestroyWindow(info.display, info.window);
			windows.remove(id);
		}
	#endif
}