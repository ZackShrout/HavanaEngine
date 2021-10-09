#include "OpenGLSurface.h"
#include "OpenGLCore.h"

namespace Havana::Graphics::OpenGL
{
    void OpenGLSurface::Present() const
    {
        glXMakeCurrent((Display*)m_window.Display(), *(Window*)m_window.Handle(), m_context);
        
        glClearColor(0.8, 0.6, 0.7, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        glXSwapBuffers((Display*)m_window.Display(), *(Window*)m_window.Handle());
    }

    void OpenGLSurface::Resize()
    {
        // TODO: implement
    }

    void OpenGLSurface::Release()
    {
        glXMakeCurrent((Display*)m_window.Display(), None, NULL);
        glXDestroyContext((Display*)m_window.Display(), m_context);
    }
}
