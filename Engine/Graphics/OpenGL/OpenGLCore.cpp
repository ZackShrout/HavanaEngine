#include "OpenGLCore.h"
#include "OpenGLSurface.h"
#define OPENGL_IMPLEMENT_LOADER // This needs to be defined here in order to call GetOpenGLExtensions();
#include "OpenGLExtensionLoader.h"
namespace Havana::Graphics::OpenGL::Core
{
    namespace
	{
		using surface_collection = Utils::free_list<OpenGLSurface>;

		surface_collection surfaces;

	} // anomymous namespace
	
	bool Initialize()
    {
		return true;
    }
	
	void Shutdown()
	{

	}

    Surface CreateSurface(Platform::Window window)
    {
        surface_id id{ surfaces.add(window) };
		
#ifdef __linux__
		// Cache a casted pointer of the display to save on casting later
		Display* display{ (Display*)window.Display() };
		
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
		GLXFBConfig *fbc { glXChooseFBConfig(display,
											DefaultScreen(display),
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
		GLXContext context { glXCreateContextAttribsARB(display, fbc[0], NULL, true,
													contextAttribs) };
		if (!context) {
			return {};
		}

		OGL::GetOpenGLExtensions();

		int major { 0 }, minor { 0 };
		glGetIntegerv(GL_MAJOR_VERSION, &major);
		glGetIntegerv(GL_MINOR_VERSION, &minor);

		surfaces[id].SetContext(context);
#endif
		return Surface{ id };
    }

	void RemoveSurface(surface_id id)
	{
		surfaces.remove(id);
	}

	void ResizeSurface(surface_id id, u32, u32)
	{
		surfaces[id].Resize();
	}

	u32 SurfaceWidth(surface_id id)
	{
		return surfaces[id].Width();
	}

	u32 SurfaceHeight(surface_id id)
	{
		return surfaces[id].Height();
	}

	void RenderSurface(surface_id id)
	{
		surfaces[id].Present();
	}
}

