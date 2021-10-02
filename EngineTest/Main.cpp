#include "Test.h"
#pragma comment(lib, "engine.lib")

#if TEST_ENTITY_COMPONENTS
#include "TestEntityComponents.h"
#elif TEST_WINDOW
#include "TestWindow.h"
#elif TEST_RENDERER
#include "TestRenderer.h"
#else
#error One of the tests must be enabled
#endif

#ifdef _WIN64
#include <Windows.h>

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
#if _DEBUG
    // MSVC Debug flags that help check for memory leaks
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

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
	EngineTest test{};

    Display* display { XOpenDisplay(NULL) };
        if (display == NULL) return 1;

    Atom wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", false);
    Atom quit_msg = XInternAtom(display, "QUIT_MSG", false);

	if (test.Initialize(display))
	{
        XEvent xev;
        bool isRunning{ true };
        while (isRunning)
        {
            while (XPending(display))
            {
                XNextEvent(display, &xev);

                switch (xev.type)
                {
                    case KeyPress:
                        
                        break;
                    case ClientMessage:
                        if ((Atom)xev.xclient.data.l[0] == wm_delete_window)
                        {      
                            //isRunning = false;
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
            // If statement here prevents test.Run() being called with no events possible and hanging the program
            if (isRunning) test.Run(display);
        }
        test.Shutdown();
        XCloseDisplay(display);
        return 0;
	}
}
#endif // _WIN64