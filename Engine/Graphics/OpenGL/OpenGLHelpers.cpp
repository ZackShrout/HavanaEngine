#include "OpenGLHelpers.h"

namespace Havana::Graphics::OpenGL::OGL
{
    #define X(ret, name, ...) name##proc * gl##name;
    GL_LIST
    GL_LIST_WIN32
    #undef X

    bool GetOpenGLExtensions()
    {

        #define X(ret, name, ...) name##proc * gl##name;
        GL_LIST
        GL_LIST_WIN32
        #undef X

        #ifdef __linux__
        void* libGL{ dlopen("libGL.so", RTLD_LAZY) };
        if (!libGL) {
            printf("libGL.so couldn't be loaded!\n");
            return false;
        }

        #define X(ret, name, ...)                                                       \
                gl##name = (name##proc *) dlsym(libGL, "gl" #name);                     \
                if (!gl##name) {                                                        \
                    printf("Function gl" #name " couldn't be loaded from libGL.so!\n"); \
                    return false;                                                       \
                }
            GL_LIST
        #undef X

    #elif _WIN32

        HINSTANCE dll{ LoadLibraryA("opengl32.dll") };
        typedef PROC WINAPI wglGetProcAddressproc(LPCSTR lpszProc);
        if (!dll) {
            OutputDebugStringA("opengl32.dll not found.\n");
            return false;
        }
        wglGetProcAddressproc* wglGetProcAddress{ (wglGetProcAddressproc*)GetProcAddress(dll, "wglGetProcAddress") };

        #define X(ret, name, ...)                                                                    \
                gl##name = (name##proc *)wglGetProcAddress("gl" #name);                                \
                if (!gl##name) {                                                                       \
                    OutputDebugStringA("Function gl" #name " couldn't be loaded from opengl32.dll\n"); \
                    return false;                                                                      \
                }
            GL_LIST
            GL_LIST_WIN32
        #undef X

    #else
        #error "OpenGL extension loading is not supported for this platform"
    #endif

        return true;
    }
}