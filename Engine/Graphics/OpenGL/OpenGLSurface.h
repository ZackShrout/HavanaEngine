#pragma once
#include "OpenGLCommonHeaders.h"

namespace Havana::Graphics::OpenGL
{
    class OpenGLSurface
    {
    public:
        explicit OpenGLSurface(Platform::Window window) : m_window{ window }
        {
            assert(m_window.Handle());
        }
    #if USE_STL_VECTOR
        // TODO: implememt
    #else
        DISABLE_COPY_AND_MOVE(OpenGLSurface);
    #endif // USE_STL_VECTOR
        ~OpenGLSurface() { Release(); }

        void Present() const;
        void Resize();
#ifdef _WIN64
        void SetContext(void* context) { m_context = context; }
#elif __linux__
        void SetContext(GLXContext context) { m_context = context; }
#endif
        constexpr u32 Width() const { return m_width; }
        constexpr u32 Height() const { return m_height; }

    private:
        Platform::Window m_window{};
#ifdef _WIN64
        void* m_context{ nullptr };
#elif __linux__
        GLXContext m_context{ nullptr };
#endif
        u32 m_width{ 0 };
        u32 m_height{ 0 };

        void Release();
    };

} // OpenGL namespace