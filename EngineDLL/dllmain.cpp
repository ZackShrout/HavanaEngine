#pragma comment(lib, "engine.lib")

// DllMain.cpp defines the entry point for the DLL application
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <crtdbg.h>

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
#if _DEBUG
        // Debug flags that help check for memory leaks
        _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
        [[fallthrough]];
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

