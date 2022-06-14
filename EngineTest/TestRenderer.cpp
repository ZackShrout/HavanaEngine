#include "TestRenderer.h"
#include "../Platforms/PlatformTypes.h"
#include "../Platforms/Platform.h"
#include "../Graphics/Renderer.h"
#include "ShaderCompilation.h"

#if TEST_RENDERER

#define USE_CONSOLE 1 // set to 1 if you want the console activated

using namespace Havana;

Graphics::RenderSurface surfaces[4];

TimeIt timer{};

bool resized{ false };
bool isRestarting{ false };
void DestroyRenderSurface(Graphics::RenderSurface &surface);
bool TestInitialize();
void TestShutdown();

#ifdef _WIN64
LRESULT WinProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	bool toggleFullscreen{ false };

	switch (msg)
	{
	case WM_DESTROY:
	{
		bool allClosed{true};
		for (u32 i{0}; i < _countof(surfaces); i++)
		{
			if (surfaces[i].window.IsValid())
			{
				if (surfaces[i].window.IsClosed())
				{
					DestroyRenderSurface(surfaces[i]);
				}
				else
				{
					allClosed = false;
				}
			}
		}
		if (allClosed && !isRestarting)
		{
			PostQuitMessage(0);
			return 0;
		}
		break;
	}
	case WM_SIZE:
		resized = (wparam != SIZE_MINIMIZED);
		break;
	case WM_SYSCHAR:
		toggleFullscreen = (wparam == VK_RETURN && (HIWORD(lparam) & KF_ALTDOWN));
		break;
	case WM_KEYDOWN:
		if (wparam == VK_ESCAPE)
		{
			PostMessage(hwnd, WM_CLOSE, 0, 0);
			return 0;
		}
		else if (wparam == VK_F11)
		{
			isRestarting = true;
			TestShutdown();
			TestInitialize();
		}
	default:
		break;
	}

	if ((resized && GetAsyncKeyState(VK_LBUTTON) >= 0) || toggleFullscreen)
	{
		Platform::Window win{ Platform::window_id{(Id::id_type)GetWindowLongPtr(hwnd, GWLP_USERDATA)} };
		for (u32 i{ 0 }; i < _countof(surfaces); i++)
		{
			if (win.GetID() == surfaces[i].window.GetID())
			{
				if (toggleFullscreen)
				{
					win.SetFullscreen(!win.IsFullscreen());
					// The default window procedure will play a system notification sound
					// when presseing the Alt+Enter keyboard combination if WM_SYSCHAR is
					// not handled. By return 0 we can tell the system that we handled
					// this message.
					return 0;
				}
				else
				{
					surfaces[i].surface.Resize(win.Width(), win.Height());
					resized = false;
				}
				break;
			}
		}
	}

	return DefWindowProc(hwnd, msg, wparam, lparam);
}

void ActivateConsole()
{
	FILE* out;
	FILE* err;
	
	AllocConsole();
	AttachConsole(GetCurrentProcessId());
	freopen_s(&out, "CON", "w", stdout);
	freopen_s(&err, "CON", "w", stderr);
	SetConsoleTitle(TEXT("Render Test"));
}


#endif // _WIN64

void CreateRenderSurface(Graphics::RenderSurface &surface, Platform::WindowInitInfo info, void* disp)
{
	surface.window = Platform::MakeWindow(&info, disp);
	surface.surface = Graphics::CreateSurface(surface.window);
}

void DestroyRenderSurface(Graphics::RenderSurface &surface)
{
	Graphics::RenderSurface temp{surface};
	surface = {};
	if (temp.surface.IsValid())
		Graphics::RemoveSurface(temp.surface.GetID());
	if (temp.window.IsValid())
		Platform::RemoveWindow(temp.window.GetID());
}

bool TestInitialize()
{
	while (!CompileShaders())
	{
		// Pop up a message box allowing the user to retry compilation.
		if (MessageBox(nullptr, L"Failed to compile engine shaders!", L"Shader Compilation Error", MB_RETRYCANCEL) != IDRETRY)
			return false;
	}

	if (!Graphics::Initialize(Graphics::GraphicsPlatform::Direct3D12)) return false;

	Platform::WindowInitInfo info[]{
		{&WinProc, nullptr, L"Render Window 1", 100, 100, 400, 800},
		{&WinProc, nullptr, L"Render Window 2", 150, 150, 800, 400},
		{&WinProc, nullptr, L"Render Window 3", 200, 200, 400, 400},
		{&WinProc, nullptr, L"Render Window 4", 250, 250, 800, 600},
	};

	static_assert(_countof(info) == _countof(surfaces));

	for (u32 i{ 0 }; i < _countof(surfaces); i++)
		CreateRenderSurface(surfaces[i], info[i], nullptr);

	isRestarting = false;
	return true;
}

void TestShutdown()
{
	for (u32 i{ 0 }; i < _countof(surfaces); i++)
		DestroyRenderSurface(surfaces[i]);

	Graphics::Shutdown();
}


#ifdef _WIN64
bool EngineTest::Initialize()
{
#if USE_CONSOLE
	ActivateConsole();
#endif // USE_CONSOLE

	return TestInitialize();
}

void EngineTest::Run()
{
	timer.Begin();

	//std::this_thread::sleep_for(std::chrono::milliseconds(10));

	for (u32 i{0}; i < _countof(surfaces); i++)
	{
		if (surfaces[i].surface.IsValid())
		{
			surfaces[i].surface.Render();
		}
	}

	timer.End();
}
#elif __linux__

bool EngineTest::Initialize(void *disp)
{
	// if (!CompileShaders())
	// {
	// 	throw std::runtime_error("Failed to compile engine shaders!");
	// 	return false;
	// }
	
	bool result{Graphics::Initialize(Graphics::GraphicsPlatform::VulkanAPI)};

	if (!result)
		return result;

	Platform::WindowInitInfo info[]{
		{nullptr, nullptr, L"Render Window 1", 100, 100, 400, 800},
		{nullptr, nullptr, L"Render Window 2", 150, 150, 800, 400},
		{nullptr, nullptr, L"Render Window 3", 200, 200, 400, 400},
		{nullptr, nullptr, L"Render Window 4", 250, 250, 800, 600},
	};

	static_assert(_countof(info) == _countof(surfaces));

	for (u32 i{0}; i < _countof(surfaces); i++)
		CreateRenderSurface(surfaces[i], info[i], disp);

	return result;
}

void EngineTest::Run(void *disp)
{
	timer.Begin();

	std::this_thread::sleep_for(std::chrono::milliseconds(10));

	for (u32 i{0}; i < _countof(surfaces); i++)
	{
		if (surfaces[i].surface.IsValid())
		{
			surfaces[i].surface.Render();
		}
	}

	timer.End();

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
				for (u32 i{ 0 }; i < _countof(surfaces); i++)
				{
					if (!surfaces[i].window.IsValid()) continue;
					if (*((Window*)surfaces[i].window.Handle()) == xev.xany.window)
					{
						if ((u32)xce.width != surfaces[i].window.Width() || (u32)xce.height != surfaces[i].window.Height())
						{
							surfaces[i].window.Resize((u32)xce.width, (u32)xce.height);
						}
					}
				}
				break;
			}
			case ClientMessage:
				if ((Atom)xev.xclient.data.l[0] == wm_delete_window)
				{      
					// Find which window was sent the close event, and call function
					for (u32 i{ 0 }; i < _countof(surfaces); i++)
					{
						if (!surfaces[i].window.IsValid()) continue;
						if (*((Window*)surfaces[i].window.Handle()) == xev.xany.window)
						{
							DestroyRenderSurface(surfaces[i]);
							break;
						}
					}

					// Check if all windows are closed, and exit application if so
					bool allClosed{ true };
					for (u32 i{ 0 }; i < _countof(surfaces); i++)
					{
						if (!surfaces[i].window.IsValid()) continue;
						if (!surfaces[i].window.IsClosed())
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
				if (xev.xkey.state == 0x18 && xev.xkey.keycode == 36)
				{
					for (u32 i{ 0 }; i < _countof(surfaces); i++)
					{
						if (!surfaces[i].window.IsValid()) continue;
						if (*((Window*)surfaces[i].window.Handle()) == xev.xany.window)
						{
							surfaces[i].window.SetFullscreen(!surfaces[i].window.IsFullscreen());
						}
					}
				}
		}
	}
}

#endif // _WIN64

void EngineTest::Shutdown()
{
	TestShutdown();
}
#endif //TEST_RENDERER