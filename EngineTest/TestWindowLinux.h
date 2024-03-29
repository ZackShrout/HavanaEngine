#pragma once

#ifdef __linux__

#include "Test.h"
#include "Platforms/PlatformTypes.h"
#include "Platforms/Platform.h"

using namespace havana;

platform::window windows[4];

enum Key
{
	ENTER = 36
};

enum State
{
	ALT = 0x18
};

class engine_test : public test
{
public:
	bool initialize() override
	{
		platform::window_init_info info[]
		{
			{ nullptr, nullptr, L"Test Window 1", 100, 100, 400, 800 },
			{ nullptr, nullptr, L"Test Window 2", 150, 150, 800, 400 },
			{ nullptr, nullptr, L"Test Window 3", 200, 200, 400, 400 },
			{ nullptr, nullptr, L"Test Window 4", 250, 250, 800, 600 },
		};

		static_assert(_countof(info) == _countof(windows));

		for (u32 i{ 0 }; i < _countof(windows); i++)
		{
			windows[i] = platform::create_window(&info[i], platform::get_display());
		}

		return true;
	}

	void run() override
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(10));

		// Open dummy window to send close msg with
		Window window = XCreateSimpleWindow(platform::get_display(), DefaultRootWindow(platform::get_display()), 0, 0, 100, 100, 0, 0, 0);
		// Set up custom client messages
		Atom wm_delete_window = XInternAtom(platform::get_display(), "WM_DELETE_WINDOW", false);
		Atom quit_msg = XInternAtom(platform::get_display(), "QUIT_MSG", false);

		XEvent xev;
		// NOTE: we use an if statement here because we are not handling all events in this translation
		//       unit, so XPending(display) will often not ever be 0, and therefore this can create
		//       an infinite loop... but this protects XNextEvent from blocking if there are no events.
		if (XPending(platform::get_display()) > 0)
		{
			XNextEvent(platform::get_display(), &xev);
			switch (xev.type)
			{
			case ConfigureNotify:
			{
				XConfigureEvent xce{ xev.xconfigure };

				// NOTE: This event is generated for a variety of reasons, so
				//		 we need to check to see which window generated the event, 
				//		 and the check if this was a window resize.
				for (u32 i{ 0 }; i < _countof(windows); ++i)
				{
					if (*((Window*)windows[i].handle()) == xev.xany.window)
					{
						if ((u32)xce.width != windows[i].width() || (u32)xce.height != windows[i].height())
						{
							windows[i].resize((u32)xce.width, (u32)xce.height);
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
						if (*((Window*)windows[i].handle()) == xev.xany.window)
						{
							windows[i].close();
						}
					}

					// Check if all windows are closed, and exit application if so
					bool all_closed{ true };
					for (u32 i{ 0 }; i < _countof(windows); ++i)
					{
						if (!windows[i].is_closed())
						{
							all_closed = false;
						}
					}
					if (all_closed)
					{
						// Set up quit message and send it using dummy window
						XEvent close;
						close.xclient.type = ClientMessage;
						close.xclient.serial = window;
						close.xclient.send_event = true;
						close.xclient.message_type = XInternAtom(platform::get_display(), "QUIT_MSG", false);
						close.xclient.format = 32;
						close.xclient.window = 0;
						close.xclient.data.l[0] = XInternAtom(platform::get_display(), "QUIT_MSG", false);
						XSendEvent(platform::get_display(), window, false, NoEventMask, &close);
					}
				}
				else
				{
					// Dont handle this here
					XPutBackEvent(platform::get_display(), &xev);
				}
				break;
			case KeyPress:
				// NOTE: "state" represents the keys held down prior to the key press the
				//		 keycode represents - the numeric evaluation is also different.
				if (xev.xkey.state == State::ALT && xev.xkey.keycode == Key::ENTER)
				{
					for (u32 i{ 0 }; i < _countof(windows); ++i)
					{
						if (*((Window*)windows[i].handle()) == xev.xany.window)
						{
							windows[i].set_fullscreen(!windows[i].is_fullscreen());
						}
					}
				}
			}
		}
	}

	void shutdown() override
	{
		for (u32 i{ 0 }; i < _countof(windows); ++i)
		{
			platform::remove_window(windows[i].get_id());
		}
	}
};

#endif // __linux__