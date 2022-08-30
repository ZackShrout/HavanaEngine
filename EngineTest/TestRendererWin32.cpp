#ifdef _WIN64

#include "Platforms/PlatformTypes.h"
#include "Platforms/Platform.h"
#include "Graphics/Renderer.h"
#include "Graphics/Direct3D12/D3D12Core.h"
#include "Content/ContentToEngine.h"
#include "Components/Entity.h"
#include "Components/Transform.h"
#include "TestRendererWin32.h"
#include "ShaderCompilation.h"
#include <filesystem>
#include <fstream>

#if TEST_RENDERER

#define USE_CONSOLE 0 // set to 1 if you want the console activated

using namespace havana;

// Multithreading test worker span code /////////////////////////////////////
#define ENABLE_TEST_WORKERS 0

constexpr u32	numThreads{ 8 };
bool			close{ false };
std::thread		workers[numThreads];

utl::vector<u8> buffer(1024 * 1024, 0);
// Test worker for upload context
void BufferTestWorker()
{
	while (!close)
	{
		auto* resource = graphics::d3d12::d3dx::create_buffer(buffer.data(), (u32)buffer.size());
		// NOTE: We can also use core::release(resource) since we're not using the buffer for rendering.
		//		 However, this is a nice test for deferred_release functionality.
		graphics::d3d12::core::deferred_release(resource);
	}
}

template<class FnPtr, class... Args>
void InitTestWorkers(FnPtr&& fnPtr, Args&&... args)
{
#if ENABLE_TEST_WORKERS
	close = false;
	for (auto& w : workers)
		w = std::thread(std::forward<FnPtr>(fnPtr), std::forward<Args>(args)...);
#endif
}

void JointTestWorkers()
{
#if ENABLE_TEST_WORKERS
	close = true;
	for (auto& w : workers) w.join();
#endif
}
/////////////////////////////////////////////////////////////////////////////

struct
{
	game_entity::entity entity{};
	graphics::camera camera{};
} camera;

id::id_type itemId{ id::invalid_id };
id::id_type modelId{ id::invalid_id };
graphics::render_surface surfaces[4];
TimeIt timer{};

bool resized{ false };
bool isRestarting{ false };
void DestroyRenderSurface(graphics::render_surface &surface);
bool TestInitialize();
void TestShutdown();
id::id_type CreateRenderItem(id::id_type entity_id);
void DestroyRenderItem(id::id_type itemId);

LRESULT win_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	bool toggleFullscreen{ false };

	switch (msg)
	{
	case WM_DESTROY:
	{
		bool allClosed{ true };
		for (u32 i{ 0 }; i < _countof(surfaces); i++)
		{
			if (surfaces[i].window.is_valid())
			{
				if (surfaces[i].window.is_closed())
				{
					DestroyRenderSurface(surfaces[i]);
				}
				else
				{
					allClosed = false;
				}
			}
		}
		if (allClosed && !isRestarting)
		{
			PostQuitMessage(0);
			return 0;
		}
		break;
	}
	case WM_SIZE:
		resized = (wparam != SIZE_MINIMIZED);
		break;
	case WM_SYSCHAR:
		toggleFullscreen = (wparam == VK_RETURN && (HIWORD(lparam) & KF_ALTDOWN));
		break;
	case WM_KEYDOWN:
		if (wparam == VK_ESCAPE)
		{
			PostMessage(hwnd, WM_CLOSE, 0, 0);
			return 0;
		}
		else if (wparam == VK_F11)
		{
			isRestarting = true;
			TestShutdown();
			TestInitialize();
		}
	default:
		break;
	}

	if ((resized && GetAsyncKeyState(VK_LBUTTON) >= 0) || toggleFullscreen)
	{
		platform::window win{ platform::window_id{(id::id_type)GetWindowLongPtr(hwnd, GWLP_USERDATA)} };
		for (u32 i{ 0 }; i < _countof(surfaces); i++)
		{
			if (win.get_id() == surfaces[i].window.get_id())
			{
				if (toggleFullscreen)
				{
					win.set_fullscreen(!win.is_fullscreen());
					// The default window procedure will play a system notification sound
					// when presseing the Alt+Enter keyboard combination if WM_SYSCHAR is
					// not handled. By return 0 we can tell the system that we handled
					// this message.
					return 0;
				}
				else
				{
					surfaces[i].surface.resize(win.width(), win.height());
					resized = false;
				}
				break;
			}
		}
	}

	return DefWindowProc(hwnd, msg, wparam, lparam);
}

game_entity::entity CreateOneGameEntity()
{
	transform::init_info transformInfo{};
	math::v3a rot{ 0, 3.14f, 0 };
	DirectX::XMVECTOR quat{ DirectX::XMQuaternionRotationRollPitchYawFromVector(DirectX::XMLoadFloat3A(&rot)) };
	math::v4a rotQuat;
	DirectX::XMStoreFloat4A(&rotQuat, quat);
	memcpy(&transformInfo.rotation[0], &rotQuat.x, sizeof(transformInfo.rotation));

	game_entity::entity_info entityInfo{};
	entityInfo.transform = &transformInfo;
	game_entity::entity ntt{ game_entity::create(entityInfo) };
	assert(ntt.is_valid());
	return ntt;
}

bool ReadFile(std::filesystem::path path, std::unique_ptr<u8[]>& data, u64& size)
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

void ActivateConsole()
{
	FILE* out;
	FILE* err;

	AllocConsole();
	AttachConsole(GetCurrentProcessId());
	freopen_s(&out, "CON", "w", stdout);
	freopen_s(&err, "CON", "w", stderr);
	SetConsoleTitle(TEXT("Render Test"));
}

void CreateRenderSurface(graphics::render_surface &surface, platform::window_init_info info, void* disp)
{
	surface.window = platform::create_window(&info, disp);
	surface.surface = graphics::create_surface(surface.window);
}

void DestroyRenderSurface(graphics::render_surface &surface)
{
	graphics::render_surface temp{ surface };
	surface = {};
	if (temp.surface.is_valid())
		graphics::remove_surface(temp.surface.get_id());
	if (temp.window.is_valid())
		platform::remove_window(temp.window.get_id());
}

bool TestInitialize()
{
	while (!CompileShaders())
	{
		// Pop up a message box allowing the user to retry compilation.
		if (MessageBox(nullptr, L"Failed to compile engine shaders!", L"Shader Compilation Error", MB_RETRYCANCEL) != IDRETRY)
			return false;
	}

	if (!graphics::initialize(graphics::graphics_platform::direct3d12)) return false;

	platform::window_init_info info[]{
		{&win_proc, nullptr, L"Render Window 1", 100, 100, 400, 800},
		{&win_proc, nullptr, L"Render Window 2", 150, 150, 800, 400},
		{&win_proc, nullptr, L"Render Window 3", 200, 200, 400, 400},
		{&win_proc, nullptr, L"Render Window 4", 250, 250, 800, 600},
	};

	static_assert(_countof(info) == _countof(surfaces));

	for (u32 i{ 0 }; i < _countof(surfaces); i++)
		CreateRenderSurface(surfaces[i], info[i], nullptr);

	// Load test model
	std::unique_ptr<u8[]> model;
	u64 size{ 0 };
	if (!ReadFile("..\\..\\enginetest\\model.model", model, size)) return false;

	modelId = content::create_resource(model.get(), content::asset_type::mesh);
	if (!id::is_valid(modelId)) return false;

	InitTestWorkers(BufferTestWorker);

	camera.entity = CreateOneGameEntity();
	camera.camera = graphics::create_camera(graphics::perspective_camera_init_info(camera.entity.get_id()));
	assert(camera.camera.is_valid());

	itemId = CreateRenderItem(CreateOneGameEntity().get_id());

	isRestarting = false;
	return true;
}

void TestShutdown()
{
	DestroyRenderItem(itemId);
	
	if (camera.camera.is_valid()) graphics::remove_camera(camera.camera.get_id());
	if (camera.entity.is_valid()) game_entity::remove(camera.entity.get_id());
	
	JointTestWorkers();

	if (id::is_valid(modelId))
	{
		content::destroy_resource(modelId, content::asset_type::mesh);
	}

	for (u32 i{ 0 }; i < _countof(surfaces); i++)
		DestroyRenderSurface(surfaces[i]);

	graphics::shutdown();
}

bool EngineTest::initialize()
{
#if USE_CONSOLE
	ActivateConsole();
#endif // USE_CONSOLE

	return TestInitialize();
}

void EngineTest::Run()
{
	timer.Begin();

	//std::this_thread::sleep_for(std::chrono::milliseconds(10));

	for (u32 i{ 0 }; i < _countof(surfaces); i++)
	{
		if (surfaces[i].surface.is_valid())
		{
			surfaces[i].surface.render();
		}
	}

	timer.End();
}

void EngineTest::shutdown()
{
	TestShutdown();
}
#endif // TEST_RENDERER
#endif // _WIN64