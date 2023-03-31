#ifdef __linux__

#include <filesystem>
#include <fstream>
#include "TestRendererLinux.h"
#include "Platforms/PlatformTypes.h"
#include "Platforms/Platform.h"
#include "Graphics/Renderer.h"
#include "Content/ContentToEngine.h"
//#include "ShaderCompilation.h"

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
graphics::render_surface _surfaces[4];
time_it timer{};

bool resized{ false };
bool is_restarting{ false };
void destroy_render_surface(graphics::render_surface &surface);
bool test_initialize();
void test_shutdown();

void
win_proc(const platform::event* const ev)
{
	switch (ev->event_type)
	{
		case platform::event::configure_notify:
		{
			// Check if any of the windows resized
			for (u32 i{ 0 }; i < _countof(_surfaces); ++i)
			{
				if (!_surfaces[i].window.is_valid()) continue;
				if (*((platform::window_handle)_surfaces[i].window.handle()) == ev->window)
				{
					if (ev->width != _surfaces[i].window.width() || ev->height != _surfaces[i].window.height())
					{
						_surfaces[i].window.resize(ev->width, ev->height);
					}
				}
			}
		}
		break;
		case platform::event::client_message:
		{
			if (platform::window_close_received(ev))
			{
				// Find which window was sent the close event, and call function
				for (u32 i{ 0 }; i < _countof(_surfaces); ++i)
				{
					if (!_surfaces[i].window.is_valid()) continue;
					if (*((platform::window_handle)_surfaces[i].window.handle()) == ev->window)
					{
						destroy_render_surface(_surfaces[i]);
						break;
					}
				}
	
				// Check if all windows are closed, and exit application if so
				bool all_closed{ true };
				for (u32 i{ 0 }; i < _countof(_surfaces); i++)
				{
					if (!_surfaces[i].window.is_valid()) continue;
					if (!_surfaces[i].window.is_closed())
					{
						all_closed = false;
					}
				}
				if (all_closed)
				{
					platform::send_quit_event();
				}
			}
		}
		break;
		case platform::event::key_press:
		{
			// NOTE: This represents "alt + enter"
			if (ev->mod_key == 0x18 && ev->keycode == 36)
			{
				for (u32 i{ 0 }; i < _countof(_surfaces); i++)
				{
					if (!_surfaces[i].window.is_valid()) continue;
					if (*((platform::window_handle)_surfaces[i].window.handle()) == ev->window)
					{
						_surfaces[i].window.set_fullscreen(!_surfaces[i].window.is_fullscreen());
					}
				}
			}
		}
	}
}

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
create_render_surface(graphics::render_surface &surface, platform::window_init_info info)
{
	surface.window = platform::create_window(&info);
	surface.surface = graphics::create_surface(surface.window);
}

void
destroy_render_surface(graphics::render_surface &surface)
{
	graphics::render_surface temp{ surface };
	surface = {};
	if (temp.surface.is_valid())
		graphics::remove_surface(temp.surface.get_id());
	if (temp.window.is_valid())
		platform::remove_window(temp.window.get_id());
}

bool
test_initialize()
{
	// if (!CompileShaders())
	// {
	// 	throw std::runtime_error("Failed to compile engine shaders!");
	// 	return false;
	// }

	if (!graphics::initialize(graphics::graphics_platform::vulkan_1)) return false;

	platform::window_init_info info[]{
		{win_proc, nullptr, L"Render Window 1", 100, 100, 400, 800},
		{win_proc, nullptr, L"Render Window 2", 150, 150, 800, 400},
		{win_proc, nullptr, L"Render Window 3", 200, 200, 400, 400},
		{win_proc, nullptr, L"Render Window 4", 250, 250, 800, 600},
	};

	static_assert(_countof(info) == _countof(_surfaces));

	for (u32 i{ 0 }; i < _countof(_surfaces); ++i)
		create_render_surface(_surfaces[i], info[i]);

	// Load test model
	std::unique_ptr<u8[]> model;
	//u64 size{ 0 };
	//if (!read_file("..\\..\\enginetest\\model.model", model, size)) return false;

	//model_id = content::create_resource(model.get(), content::asset_type::mesh);
	//if (!id::is_valid(model_id)) return false;

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

	for (u32 i{ 0 }; i < _countof(_surfaces); ++i)
		destroy_render_surface(_surfaces[i]);

	graphics::shutdown();
}

bool
engine_test::initialize()
{
	return test_initialize() ;
}

void
engine_test::run()
{
	timer.begin();

	std::this_thread::sleep_for(std::chrono::milliseconds(10));

	for (u32 i{ 0 }; i < _countof(_surfaces); ++i)
	{
		if (_surfaces[i].surface.is_valid())
		{
			graphics::frame_info info{};
			
			_surfaces[i].surface.render(info);
		}
	}

	timer.end();
}

void
engine_test::shutdown()
{
	test_shutdown();
}
#endif //TEST_RENDERER

#endif // __linux__