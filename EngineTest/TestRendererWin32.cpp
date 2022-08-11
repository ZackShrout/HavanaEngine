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

using namespace Havana;

// Multithreading test worker span code /////////////////////////////////////
#define ENABLE_TEST_WORKERS 0

constexpr u32	numThreads{ 8 };
bool			close{ false };
std::thread		workers[numThreads];

Utils::vector<u8> buffer(1024 * 1024, 0);
// Test worker for upload context
void BufferTestWorker()
{
	while (!close)
	{
		auto* resource = Graphics::D3D12::D3DX::CreateBuffer(buffer.data(), (u32)buffer.size());
		// NOTE: We can also use Core::Release(resource) since we're not using the buffer for rendering.
		//		 However, this is a nice test for DeferredRelease functionality.
		Graphics::D3D12::Core::DeferredRelease(resource);
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
Entity::Entity entity{};
Id::id_type modelId{ Id::INVALID_ID };
Graphics::Camera camera{};
Graphics::RenderSurface surfaces[4];
TimeIt timer{};

bool resized{ false };
bool isRestarting{ false };
void DestroyRenderSurface(Graphics::RenderSurface &surface);
bool TestInitialize();
void TestShutdown();

LRESULT WinProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	bool toggleFullscreen{ false };

	switch (msg)
	{
	case WM_DESTROY:
	{
		bool allClosed{ true };
		for (u32 i{ 0 }; i < _countof(surfaces); i++)
		{
			if (surfaces[i].window.IsValid())
			{
				if (surfaces[i].window.IsClosed())
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
		Platform::Window win{ Platform::window_id{(Id::id_type)GetWindowLongPtr(hwnd, GWLP_USERDATA)} };
		for (u32 i{ 0 }; i < _countof(surfaces); i++)
		{
			if (win.GetID() == surfaces[i].window.GetID())
			{
				if (toggleFullscreen)
				{
					win.SetFullscreen(!win.IsFullscreen());
					// The default window procedure will play a system notification sound
					// when presseing the Alt+Enter keyboard combination if WM_SYSCHAR is
					// not handled. By return 0 we can tell the system that we handled
					// this message.
					return 0;
				}
				else
				{
					surfaces[i].surface.Resize(win.Width(), win.Height());
					resized = false;
				}
				break;
			}
		}
	}

	return DefWindowProc(hwnd, msg, wparam, lparam);
}

Entity::Entity CreateOneGameEntity()
{
	Transform::InitInfo transformInfo{};
	Math::Vec3A rot{ 0, 3.14f, 0 };
	DirectX::XMVECTOR quat{ DirectX::XMQuaternionRotationRollPitchYawFromVector(DirectX::XMLoadFloat3A(&rot)) };
	Math::Vec4A rotQuat;
	DirectX::XMStoreFloat4A(&rotQuat, quat);
	memcpy(&transformInfo.rotation[0], &rotQuat.x, sizeof(transformInfo.rotation));

	Entity::EntityInfo entityInfo{};
	entityInfo.transform = &transformInfo;
	Entity::Entity ntt{ Entity::CreateEntity(entityInfo) };
	assert(ntt.IsValid());
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

void CreateRenderSurface(Graphics::RenderSurface &surface, Platform::WindowInitInfo info, void* disp)
{
	surface.window = Platform::MakeWindow(&info, disp);
	surface.surface = Graphics::CreateSurface(surface.window);
}

void DestroyRenderSurface(Graphics::RenderSurface &surface)
{
	Graphics::RenderSurface temp{ surface };
	surface = {};
	if (temp.surface.IsValid())
		Graphics::RemoveSurface(temp.surface.GetID());
	if (temp.window.IsValid())
		Platform::RemoveWindow(temp.window.GetID());
}

bool TestInitialize()
{
	while (!CompileShaders())
	{
		// Pop up a message box allowing the user to retry compilation.
		if (MessageBox(nullptr, L"Failed to compile engine shaders!", L"Shader Compilation Error", MB_RETRYCANCEL) != IDRETRY)
			return false;
	}

	if (!Graphics::Initialize(Graphics::GraphicsPlatform::Direct3D12)) return false;

	Platform::WindowInitInfo info[]{
		{&WinProc, nullptr, L"Render Window 1", 100, 100, 400, 800},
		{&WinProc, nullptr, L"Render Window 2", 150, 150, 800, 400},
		{&WinProc, nullptr, L"Render Window 3", 200, 200, 400, 400},
		{&WinProc, nullptr, L"Render Window 4", 250, 250, 800, 600},
	};

	static_assert(_countof(info) == _countof(surfaces));

	for (u32 i{ 0 }; i < _countof(surfaces); i++)
		CreateRenderSurface(surfaces[i], info[i], nullptr);

	// Load test model
	std::unique_ptr<u8[]> model;
	u64 size{ 0 };
	if (!ReadFile("..\\..\\enginetest\\model.model", model, size)) return false;

	modelId = Content::CreateResource(model.get(), Content::AssetType::Mesh);
	if (!Id::IsValid(modelId)) return false;

	InitTestWorkers(BufferTestWorker);

	entity = CreateOneGameEntity();
	camera = Graphics::CreateCamera(Graphics::PerspectiveCameraInitInfo(entity.GetID()));
	assert(camera.IsValid());

	isRestarting = false;
	return true;
}

void TestShutdown()
{
	if (camera.IsValid()) Graphics::RemoveCamera(camera.GetID());
	if (entity.IsValid()) Entity::RemoveEntity(entity.GetID());
	
	JointTestWorkers();

	if (Id::IsValid(modelId))
	{
		Content::DestroyResource(modelId, Content::AssetType::Mesh);
	}

	for (u32 i{ 0 }; i < _countof(surfaces); i++)
		DestroyRenderSurface(surfaces[i]);

	Graphics::Shutdown();
}

bool EngineTest::Initialize()
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
		if (surfaces[i].surface.IsValid())
		{
			surfaces[i].surface.Render();
		}
	}

	timer.End();
}

void EngineTest::Shutdown()
{
	TestShutdown();
}
#endif // TEST_RENDERER
#endif // _WIN64