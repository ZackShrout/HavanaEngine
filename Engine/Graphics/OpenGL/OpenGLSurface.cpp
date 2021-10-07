#include "OpenGLSurface.h"

namespace Havana::Graphics::OpenGL
{
    void OpenGLSurface::Present() const
    {
        glClearColor(0.8, 0.6, 0.7, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        glXSwapBuffers(m_display, m_window);
    }

    void OpenGLSurface::Resize();
    {
        // TODO: implement
    }
}
