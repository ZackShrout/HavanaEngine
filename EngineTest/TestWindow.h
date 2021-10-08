#pragma once

#include "Test.h"
#include "../Platforms/PlatformTypes.h"
#include "../Platforms/Platform.h"

using namespace Havana;

Platform::Window windows[4];

#ifdef _WIN64
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
enum Key
{
	ENTER = 36
};

enum State
{
	ALT = 0x18
};
#endif // _WIN64

class EngineTest : public Test
{
public:
	bool Initialize(void* display) override
	{
		Platform::WindowInitInfo info[]
		{
			{ nullptr, nullptr, L"Test Window 1", 100, 100, 400, 800 },
			{ nullptr, nullptr, L"Test Window 2", 150, 150, 800, 400 },
			{ nullptr, nullptr, L"Test Window 3", 200, 200, 400, 400 },
			{ nullptr, nullptr, L"Test Window 4", 250, 250, 800, 600 },
		};

		static_assert(_countof(info) == _countof(windows));

		for (u32 i{ 0 }; i < _countof(windows); i++)
		{
			windows[i] = Platform::MakeWindow(&info[i], display);
		}

		return true;
	}
	
#ifdef _WIN64
	void Run() override
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
#elif __linux__
	void Run(void* disp) override
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(10));

		// Cache a casted pointer of the display to save on casting later
		Display* display{ (Display*)disp };
		// Open dummy window to send close msg with
		Window window = XCreateSimpleWindow(display, DefaultRootWindow(display), 0, 0, 100, 100, 0, 0, 0);
		// Set up custom client messages
		Atom wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", false);
		Atom quit_msg = XInternAtom(display, "QUIT_MSG", false);

		XEvent xev;
		// NOTE: we use an if statement here because we are not handling all events in this translation
		//       unit, so XPending(display) will often not ever be 0, and therefore this can create
		//       an infinite loop... but this protects XNextEvent from blocking if there are no events.
		if (XPending(display) > 0)
		{
			XNextEvent(display, &xev);
			switch (xev.type)
			{
				case ConfigureNotify:
				{
					XConfigureEvent xce{ xev.xconfigure };

					// NOTE: This event is generated for a variety of reasons, so
					//		 we need to check to see which window generated the event, 
					//		 and the check if this was a window resize.
					for (u32 i{ 0 }; i < _countof(windows); i++)
					{
						if (*((Window*)windows[i].Handle()) == xev.xany.window)
						{
							if ((u32)xce.width != windows[i].Width() || (u32)xce.height != windows[i].Height())
							{
								windows[i].Resize((u32)xce.width, (u32)xce.height);
							}
						}
					}
					break;
				}
				case ClientMessage:
					if ((Atom)xev.xclient.data.l[0] == wm_delete_window)
					{      
						// Find which window was sent the close event, and call function
						for (u32 i{ 0 }; i < _countof(windows); i++)
						{
							if (*((Window*)windows[i].Handle()) == xev.xany.window)
							{
								windows[i].Close();
							}
						}

						// Check if all windows are closed, and exit application if so
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
							// Set up quit message and send it using dummy window
							XEvent close;
							close.xclient.type = ClientMessage;
							close.xclient.serial = window;
							close.xclient.send_event = true;
							close.xclient.message_type = XInternAtom(display, "QUIT_MSG", false);
							close.xclient.format = 32;
							close.xclient.window = 0;
							close.xclient.data.l[0] = XInternAtom(display, "QUIT_MSG", false);
							XSendEvent(display, window, false, NoEventMask, &close);
						}
					}
					else
					{
						// Dont handle this here
                        XPutBackEvent(display, &xev);
					}
					break;
				case KeyPress:
					// NOTE: "state" represents the keys held down prior to the key press the
					//		 keycode represents - the numeric evaluation is also different.
					if (xev.xkey.state == State::ALT && xev.xkey.keycode == Key::ENTER)
					{
						for (u32 i{ 0 }; i < _countof(windows); i++)
						{
							if (*((Window*)windows[i].Handle()) == xev.xany.window)
							{
								windows[i].SetFullscreen(!windows[i].IsFullscreen());
							}
						}
					}
			}
		}
	}
#endif

	void Shutdown() override
	{
		for (u32 i{ 0 }; i < _countof(windows); i++)
		{
			Platform::RemoveWindow(windows[i].GetID());
		}
	}
private:

};