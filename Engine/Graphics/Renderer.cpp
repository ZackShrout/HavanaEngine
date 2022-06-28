#include "Renderer.h"
#include "GraphicsPlatformInterface.h"
#include "../Graphics/Direct3D12/D3D12Interface.h"
#include "../Graphics/Vulkan/VulkanInterface.h"
#include "../Graphics/OpenGL/OpenGLInterface.h"

namespace Havana::Graphics
{
	namespace
	{
		// Defines where the compiled engine shaders files is located for each one of the supported APIs.
		constexpr const char* engineShaderPaths[]
		{
			"./Shaders/D3D12/shaders.bin",
			"./Shaders/Vulkan/shaders.bin"
		};
		
		PlatformInterface gfx{};

		bool SetGraphicsPlatform(GraphicsPlatform platform)
		{
			switch (platform)
			{
			#ifdef _WIN64
			case GraphicsPlatform::Direct3D12:
				D3D12::GetPlatformInterface(gfx);
				break;
			#endif // _WIN64
			case GraphicsPlatform::VulkanAPI:
				Vulkan::GetPlatformInterface(gfx);
				break;
			case GraphicsPlatform::OpenGraphicsL:
				OpenGL::GetPlatformInterface(gfx);
				break;
			default:
				return false;
			}
			assert(gfx.platform == platform);
			return true;
		}

	} // anonymous namespace

	bool Initialize(GraphicsPlatform platform)
	{
		return SetGraphicsPlatform(platform) && gfx.Initialize();
	}

	void Shutdown()
	{
		if(gfx.platform != (GraphicsPlatform) - 1) gfx.Shutdown();
	}

	const char* GetEngineShadersPath()
	{
		return engineShaderPaths[(u32)gfx.platform];
	}

	const char* GetEngineShadersPath(GraphicsPlatform platform)
	{
		return engineShaderPaths[(u32)platform];
	}

	Surface CreateSurface(Platform::Window window)
	{
		return gfx.Surface.Create(window);
	}

	void RemoveSurface(surface_id id)
	{
		assert(Id::IsValid(id));
		gfx.Surface.Remove(id);
	}

	void Surface::Resize(u32 width, u32 height) const
	{
		assert(IsValid());
		return gfx.Surface.Resize(m_id, width, height);
	}

	u32 Surface::Width() const
	{
		assert(IsValid());
		return gfx.Surface.Width(m_id);
	}

	u32 Surface::Height() const
	{
		assert(IsValid());
		return gfx.Surface.Height(m_id);
	}

	void Surface::Render() const
	{
		assert(IsValid());
		gfx.Surface.Render(m_id);
	}

	Id::id_type AddSubmesh(const u8*& data)
	{
		return gfx.Resources.AddSubmesh(data);
	}

	void RemoveSubmesh(Id::id_type id)
	{
		gfx.Resources.RemoveSubmesh(id);
	}
}