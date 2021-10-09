#include "OpenGLCore.h"
#include "OpenGLHelpers.h"

namespace Havana::Graphics::OpenGL::Core
{
    bool Initialize()
    {

    }

    Surface CreateSurface(Platform::Window window)
    {
        // Create_the_modern_OpenGL_context
		static int visualAttribs[] {
			GLX_RENDER_TYPE, GLX_RGBA_BIT,
			GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
			GLX_DOUBLEBUFFER, true,
			GLX_RED_SIZE, 1,
			GLX_GREEN_SIZE, 1,
			GLX_BLUE_SIZE, 1,
			None
		};

		int numFBC { 0 };
		GLXFBConfig *fbc { glXChooseFBConfig(window.Display(),
											DefaultScreen(window.Display()),
											visualAttribs, &numFBC) };
		if (!fbc) {
			return {};
		}

		OGL::glXCreateContextAttribsARBProc glXCreateContextAttribsARB { 0 };
		glXCreateContextAttribsARB =
			(OGL::glXCreateContextAttribsARBProc)
			glXGetProcAddress((const GLubyte*)"glXCreateContextAttribsARB");

		if (!glXCreateContextAttribsARB) {
			return {};
		}

		// Set desired minimum OpenGL version
		int contextAttribs[] {
			GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
			GLX_CONTEXT_MINOR_VERSION_ARB, 2,
			None
		};

		// Create modern OpenGL context
		GLXContext context { glXCreateContextAttribsARB(window.Display(), fbc[0], NULL, true,
													contextAttribs) };
		if (!context) {
			return {};
		}

		OGL::GetOpenGLExtensions();

		glXMakeCurrent(window.Display(), window.Handle(), context);

		int major { 0 }, minor { 0 };
		glGetIntegerv(GL_MAJOR_VERSION, &major);
		glGetIntegerv(GL_MINOR_VERSION, &minor);
    }
}

