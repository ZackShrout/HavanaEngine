#include "Test.h"
#pragma comment(lib, "engine.lib")

#if TEST_ENTITY_COMPONENTS
#include "TestEntityComponents.h"
#elif TEST_WINDOW
#include "TestWindowWin32.h"
#include "TestWindowLinux.h"
#elif TEST_RENDERER
#include "TestRenderer.h"
#else
#error One of the tests must be enabled
#endif

#ifdef _WIN64
#include <Windows.h>
#include <filesystem>

// TODO: This is a duplicate
std::filesystem::path SetCurrentDirectoryToExecutablePath()
{
    // set the working directory to the executable path
    wchar_t path[MAX_PATH];
    const uint32_t length{ GetModuleFileName(0, &path[0], MAX_PATH) };

    if (!length || GetLastError() == ERROR_INSUFFICIENT_BUFFER) return {};

    std::filesystem::path p{ path };
    std::filesystem::current_path(p.parent_path());

    return std::filesystem::current_path();
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
#if _DEBUG
    // MSVC Debug flags that help check for memory leaks
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    SetCurrentDirectoryToExecutablePath();
    EngineTest test{};

    if (test.Initialize())
    {
        MSG msg;
        bool isRunning{ true };
        while (isRunning)
        {
            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
                isRunning &= (msg.message != WM_QUIT);
            }
            test.Run();
        }
        test.Shutdown();
        return 0;
    }
}

#elif __linux__
#include <X11/Xlib.h>

int main(int argc, char* argv[])
{
	XInitThreads();
    
    EngineTest test{};

    // Open an X server connection
    Display* display { XOpenDisplay(NULL) };
        if (display == NULL) return 1;
    
    // Set up custom client messages
    Atom wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", false);
    Atom quit_msg = XInternAtom(display, "QUIT_MSG", false);

	if (test.Initialize(display))
	{
        XEvent xev;
        bool isRunning{ true };
        while (isRunning)
        {
            // NOTE: we use an if statement here because we are not handling all events in this translation
            //       unit, so XPending(display) will often not ever be 0, and therefore this can create
            //       an infinite loop... but this protects XNextEvent from blocking if there are no events.
            if (XPending(display) > 0)
            {
                XNextEvent(display, &xev);

                switch (xev.type)
                {
                    case KeyPress:
                        
                        break;
                    case ClientMessage:
                        if ((Atom)xev.xclient.data.l[0] == wm_delete_window)
                        {
                            // Dont handle this here
                            XPutBackEvent(display, &xev);
                        }
                        if ((Atom)xev.xclient.data.l[0] == quit_msg)
                        {
                            isRunning = false;
                        }
                        break;
                }
            }
            test.Run(display);
        }
        test.Shutdown();
        XCloseDisplay(display);
        return 0;
	}
}
#endif // platforms