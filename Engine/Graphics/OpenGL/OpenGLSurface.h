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
        void SetContext(GLXContext context) { m_context = context; }
        constexpr u32 Width() const { return m_width; }
        constexpr u32 Height() const { return m_height; }

    private:
        Platform::Window m_window{};
        GLXContext m_context{ nullptr };
        u32 m_width{ 0 };
        u32 m_height{ 0 };

        void Release();
    };

} // OpenGL namespace