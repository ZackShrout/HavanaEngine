//**************
// Entry point
//**************

#ifdef _WIN64

#ifndef WIN_32_LEAN_AND_MEAN
#define WIN_32_LEAN_AND_MEAN
#endif // WIN_32_LEAN_AND_MEAN

#include <Windows.h>
#include <crtdbg.h>

#ifndef USE_WITH_EDITOR

extern bool EngineInitialize();
extern void EngineUpdate();
extern void EngineShutdown();

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
#if _DEBUG
    // Debug flags that help check for memory leaks
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
    if (EngineInitialize())
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
                EngineUpdate();
            }
        }
        EngineShutdown();
        return 0;
    }
}

#endif // !USE_WITH_EDITOR
#endif // _WIN64
