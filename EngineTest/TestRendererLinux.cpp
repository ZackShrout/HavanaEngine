#ifdef __linux__

#include <filesystem>
#include <fstream>
#include "TestRendererLinux.h"
#include "Platforms/PlatformTypes.h"
#include "Platforms/Platform.h"
#include "Graphics/Renderer.h"
#include "Graphics/Direct3D12/D3D12Core.h"
#include "Content/ContentToEngine.h"
#include "ShaderCompilation.h"

#if TEST_RENDERER

using namespace havana;

// Multithreading test worker span code /////////////////////////////////////
#define ENABLE_TEST_WORKERS 0

constexpr u32	num_threads{ 8 };
bool			close{ false };
std::thread		workers[num_threads];

utl::vector<u8> buffer(1024 * 1024, 0);
// Test worker for upload context
void buffer_test_worker()
{
	//while (!close)
	//{
	//	auto* resource = Graphics::d3d12::d3dx::CreateBuffer(buffer.data(), (u32)buffer.size());
	//	// NOTE: We can also use core::release(resource) since we're not using the buffer for rendering.
	//	//		 However, this is a nice test for deferred_release functionality.
	//	Graphics::d3d12::core::deferred_release(resource);
	//}
}

template<class FnPtr, class... Args>
void init_test_workers(FnPtr&& fnPtr, Args&&... args)
{
#if ENABLE_TEST_WORKERS
	close = false;
	for (auto& w : workers)
		w = std::thread(std::forward<FnPtr>(fnPtr), std::forward<Args>(args)...);
#endif
}

void joint_test_workers()
{
#if ENABLE_TEST_WORKERS
	close = true;
	for (auto& w : workers) w.join();
#endif
}
/////////////////////////////////////////////////////////////////////////////

id::id_type model_id{ id::invalid_id };
Graphics::render_surface surfaces[4];
time_it timer{};

bool resized{ false };
bool is_restarting{ false };
void destroy_render_surface(graphics::render_surface &surface);
bool test_initialize();
void test_shutdown();

bool
read_file(std::filesystem::path path, std::unique_ptr<u8[]>& data, u64& size)
{
	if (!std::filesystem::exists(path)) return false;

	size = std::filesystem::file_size(path);
	assert(size);
	if (!size) return false;

	data = std::make_unique<u8[]>(size);
	std::ifstream file{ path, std::ios::in | std::ios::binary };
	if (!file || !file.read((char*)data.get(), size))
	{
		file.close();
		return false;
	}

	file.close();
	return true;
}

void
create_render_surface(Graphics::render_surface &surface, Platform::window_init_info info, void* disp)
{
	surface.window = platform::create_window(&info, disp);
	surface.surface = graphics::create_surface(surface.window);
}

void
destroy_render_surface(Graphics::render_surface &surface)
{
	graphics::render_surface temp{ surface };
	surface = {};
	if (temp.surface.is_valid())
		graphics::remove_surface(temp.surface.get_id());
	if (temp.window.is_valid())
		platform::remove_window(temp.window.get_id());
}

bool
test_initialize(void *disp)
{
	// if (!CompileShaders())
	// {
	// 	throw std::runtime_error("Failed to compile engine shaders!");
	// 	return false;
	// }

	if (!graphics::initialize(graphics::graphics_platform::vulkan_1)) return false;

	platform::window_init_info info[]{
		{nullptr, nullptr, L"Render Window 1", 100, 100, 400, 800},
		{nullptr, nullptr, L"Render Window 2", 150, 150, 800, 400},
		{nullptr, nullptr, L"Render Window 3", 200, 200, 400, 400},
		{nullptr, nullptr, L"Render Window 4", 250, 250, 800, 600},
	};

	static_assert(_countof(info) == _countof(surfaces));

	for (u32 i{ 0 }; i < _countof(surfaces); ++i)
		create_render_surface(surfaces[i], info[i], disp);

	// Load test model
	std::unique_ptr<u8[]> model;
	u64 size{ 0 };
	if (!read_file("..\\..\\enginetest\\model.model", model, size)) return false;

	model_id = content::create_resource(model.get(), content::asset_type::mesh);
	if (!id::is_valid(model_id)) return false;

	init_test_workers(buffer_test_worker);

	is_restarting = false;
	return true;
}

void
test_shutdown()
{
	joint_test_workers();

	if (id::is_valid(model_id))
	{
		content::destroy_resource(model_id, content::asset_type::mesh);
	}

	for (u32 i{ 0 }; i < _countof(surfaces); ++i)
		destroy_render_surface(surfaces[i]);

	graphics::shutdown();
}

bool
engine_test::initialize(void *disp)
{
	return test_initialize(disp);
}

void
engine_test::run(void *disp)
{
	timer.begin();

	std::this_thread::sleep_for(std::chrono::milliseconds(10));

	for (u32 i{ 0 }; i < _countof(surfaces); ++i)
	{
		if (surfaces[i].surface.is_valid())
		{
			surfaces[i].surface.render();
		}
	}

	timer.end();

	// Cache a casted pointer of the display to save on casting later
	Display* display{ (Display*)disp };
	// Open dummy window to send close msg with
	Window window = XCreateSimpleWindow(display, DefaultRootWindow(display), 0, 0, 100, 100, 0, 0, 0);
	// Set up custom client messages
	Atom wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", false);
	Atom quit_msg = XInternAtom(display, "QUIT_MSG", false);

	XEvent xev;
	// NOTE: we use an if statement here because we are not handling all events in this translation
	//       unit, so XPending(display) will often not ever be 0, and therefore this can create
	//       an infinite loop... but this protects XNextEvent from blocking if there are no events.
	if (XPending(display) > 0)
	{
		XNextEvent(display, &xev);
		switch (xev.type)
		{
		case ConfigureNotify:
		{
			XConfigureEvent xce{ xev.xconfigure };

			// NOTE: This event is generated for a variety of reasons, so
			//		 we need to check to see which window generated the event, 
			//		 and the check if this was a window resize.
			for (u32 i{ 0 }; i < _countof(surfaces); ++i)
			{
				if (!surfaces[i].window.is_valid()) continue;
				if (*((Window*)surfaces[i].window.Handle()) == xev.xany.window)
				{
					if ((u32)xce.width != surfaces[i].window.width() || (u32)xce.height != surfaces[i].window.height())
					{
						surfaces[i].window.resize((u32)xce.width, (u32)xce.height);
					}
				}
			}
			break;
		}
		case ClientMessage:
			if ((Atom)xev.xclient.data.l[0] == wm_delete_window)
			{
				// Find which window was sent the close event, and call function
				for (u32 i{ 0 }; i < _countof(surfaces); ++i)
				{
					if (!surfaces[i].window.is_valid()) continue;
					if (*((Window*)surfaces[i].window.Handle()) == xev.xany.window)
					{
						destroy_render_surface(surfaces[i]);
						break;
					}
				}

				// Check if all windows are closed, and exit application if so
				bool all_closed{ true };
				for (u32 i{ 0 }; i < _countof(surfaces); i++)
				{
					if (!surfaces[i].window.is_valid()) continue;
					if (!surfaces[i].window.is_closed())
					{
						all_closed = false;
					}
				}
				if (all_closed)
				{
					// Set up quit message and send it using dummy window
					XEvent close;
					close.xclient.type = ClientMessage;
					close.xclient.serial = window;
					close.xclient.send_event = true;
					close.xclient.message_type = XInternAtom(display, "QUIT_MSG", false);
					close.xclient.format = 32;
					close.xclient.window = 0;
					close.xclient.data.l[0] = XInternAtom(display, "QUIT_MSG", false);
					XSendEvent(display, window, false, NoEventMask, &close);
				}
			}
			else
			{
				// Dont handle this here
				XPutBackEvent(display, &xev);
			}
			break;
		case KeyPress:
			// NOTE: "state" represents the keys held down prior to the key press the
			//		 keycode represents - the numeric evaluation is also different.
			if (xev.xkey.state == 0x18 && xev.xkey.keycode == 36)
			{
				for (u32 i{ 0 }; i < _countof(surfaces); i++)
				{
					if (!surfaces[i].window.is_valid()) continue;
					if (*((Window*)surfaces[i].window.Handle()) == xev.xany.window)
					{
						surfaces[i].window.set_fullscreen(!surfaces[i].window.is_fullscreen());
					}
				}
			}
		}
	}
}

void
engine_test::shutdown()
{
	test_shutdown();
}
#endif //TEST_RENDERER

#endif // __linux__