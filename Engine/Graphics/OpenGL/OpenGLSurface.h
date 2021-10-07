#pragma once
#include "OpenGLCommonHeaders.h"

namespace Havana::Graphics::OpenGL
{
    class OpenGLSurface
    {
    public:
    #ifdef __linux__
        explicit OpenGLSurface(Platform::Window window, Display* display) : m_window{ window }, m_display{ display }
        {
            assert(m_window.Handle());
            assert(m_display));
        }
    #elif _WIN64
        explicit OpenGLSurface(Platform::Window window) : m_window{ window }
        {
            assert(m_window.Handle());
        }
    #endif // __linux__
    #if USE_STL_VECTOR
        // TODO: implememt
    #else
        DISABLE_COPY_AND_MOVE(OpenGLSurface);
    #endif // USE_STL_VECTOR
        ~OpenGLSurface() { Release(); }

        void Present() const;
        void Resize();
        constexpr u32 Width() const { return m_width; }
        constexpr u32 Height() const { return m_height; }

    private:
        Platform::Window m_window{};
    #ifdef __linux__
        Display* m_display{};
    #endif // __linux__
        u32 m_width{ 0 };
        u32 m_height{ 0 };

        Release();

    };

} // OpenGL namespace