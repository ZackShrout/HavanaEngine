#include "Test.h"
#pragma comment(lib, "engine.lib")

// What test are we performing?
#define TEST_ENTITY_COMPONENTS 0
#define TEST_WINDOW 1

#if TEST_ENTITY_COMPONENTS
#include "TestEntityComponents.h"
#elif TEST_WINDOW
#include "TestWindow.h"
#else
#error One of the tests must be enabled
#endif

#ifdef _WIN64
#include <Windows.h>

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
#if _DEBUG
    // Debug flags that help check for memory leaks
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

#else
int main()
{

#if _DEBUG
	// Debug flags that help check for memory leaks
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	EngineTest test{};

	if (test.Initialize())
	{
		test.Run();
	}

	test.Shutdown();

	return 0;
}
#endif // _WIN64