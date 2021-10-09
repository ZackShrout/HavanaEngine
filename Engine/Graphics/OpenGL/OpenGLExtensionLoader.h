#pragma once
#include "OpenGLCommonHeaders.h"

namespace Havana::Graphics::OpenGL::OGL
{
    #ifdef __linux__
    #include <dlfcn.h>
    // Function pointer needed to get X11 context attributes
    using glXCreateContextAttribsARBProc = 
        GLXContext (*)(Display*, GLXFBConfig, GLXContext, Bool, const int*);
    #define GLDECL // Not needed for Linux
    #define GL_LIST_WIN32 // Not needed for Linux
    #endif // __linux__

    #ifdef _WIN32
    #ifndef NOMINMAX
    #define NOMINMAX
    #endif // !NOMINMAX
    #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
    #endif // !WIN32_LEAN_AND_MEAN
    #include <windows.h>
    #define GLDECL CALLBACK

    #define GL_ARRAY_BUFFER                   0x8892
    #define GL_ARRAY_BUFFER_BINDING           0x8894
    #define GL_COLOR_ATTACHMENT0              0x8CE0
    #define GL_COMPILE_STATUS                 0x8B81
    #define GL_CURRENT_PROGRAM                0x8B8D
    #define GL_DYNAMIC_DRAW                   0x88E8
    #define GL_ELEMENT_ARRAY_BUFFER           0x8893
    #define GL_ELEMENT_ARRAY_BUFFER_BINDING   0x8895
    #define GL_FRAGMENT_SHADER                0x8B30
    #define GL_FRAMEBUFFER                    0x8D40
    #define GL_FRAMEBUFFER_COMPLETE           0x8CD5
    #define GL_FUNC_ADD                       0x8006
    #define GL_INVALID_FRAMEBUFFER_OPERATION  0x0506
    #define GL_MAJOR_VERSION                  0x821B
    #define GL_MINOR_VERSION                  0x821C
    #define GL_STATIC_DRAW                    0x88E4
    #define GL_STREAM_DRAW                    0x88E0
    #define GL_TEXTURE0                       0x84C0
    #define GL_VERTEX_SHADER                  0x8B31

    typedef char GLchar;
    typedef ptrdiff_t GLintptr;
    typedef ptrdiff_t GLsizeiptr;

    #define GL_LIST_WIN32\
        X(void,      BlendEquation,           GLenum mode)\
        X(void,      ActiveTexture,           GLenum texture)
    #endif // _WIN32

    #include <GL/gl.h>

    // List of OpenGL extntion (modern) functions to load. This list can be modified as needed
    #define GL_LIST\
        X(void,      AttachShader,            GLuint program, GLuint shader)\
        X(void,      BindBuffer,              GLenum target, GLuint buffer)\
        X(void,      BindFramebuffer,         GLenum target, GLuint framebuffer)\
        X(void,      BufferData,              GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage)\
        X(void,      BufferSubData,           GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid * data)\
        X(GLenum,    CheckFramebufferStatus,  GLenum target)\
        X(void,      ClearBufferfv,           GLenum buffer, GLint drawbuffer, const GLfloat * value)\
        X(void,      CompileShader,           GLuint shader)\
        X(GLuint,    CreateProgram,           void)\
        X(GLuint,    CreateShader,            GLenum type)\
        X(void,      DeleteBuffers,           GLsizei n, const GLuint *buffers)\
        X(void,      DeleteFramebuffers,      GLsizei n, const GLuint *framebuffers)\
        X(void,      EnableVertexAttribArray, GLuint index)\
        X(void,      DrawBuffers,             GLsizei n, const GLenum *bufs)\
        X(void,      FramebufferTexture2D,    GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)\
        X(void,      GenBuffers,              GLsizei n, GLuint *buffers)\
        X(void,      GenFramebuffers,         GLsizei n, GLuint * framebuffers)\
        X(GLint,     GetAttribLocation,       GLuint program, const GLchar *name)\
        X(void,      GetShaderInfoLog,        GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog)\
        X(void,      GetShaderiv,             GLuint shader, GLenum pname, GLint *params)\
        X(GLint,     GetUniformLocation,      GLuint program, const GLchar *name)\
        X(void,      LinkProgram,             GLuint program)\
        X(void,      ShaderSource,            GLuint shader, GLsizei count, const GLchar* const *string, const GLint *length)\
        X(void,      Uniform1i,               GLint location, GLint v0)\
        X(void,      Uniform1f,               GLint location, GLfloat v0)\
        X(void,      Uniform2f,               GLint location, GLfloat v0, GLfloat v1)\
        X(void,      Uniform4f,               GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)\
        X(void,      UniformMatrix4fv,        GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)\
        X(void,      UseProgram,              GLuint program)\
        X(void,      VertexAttribPointer,     GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid * pointer)

    #define X(ret, name, ...) typedef ret GLDECL name##proc(__VA_ARGS__); extern name##proc * gl##name;
    GL_LIST
    GL_LIST_WIN32
    #undef X

    // Call this to "initialize" OpenGL functions, after getting an OpenGL context
    bool GetOpenGLExtensions();

    // NOTE: This must be defined in exactly one CPP file (and only one) in order for the
    //       GetOpenGLExtensions() call to to be defined and work at runtime.
    #ifdef OPENGL_IMPLEMENT_LOADER

    #define X(ret, name, ...) name##proc * gl##name;
    GL_LIST
    GL_LIST_WIN32
    #undef X

    bool GetOpenGLExtensions()
    {
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

    #endif // OPEN_GL_EXT_LOADER
}