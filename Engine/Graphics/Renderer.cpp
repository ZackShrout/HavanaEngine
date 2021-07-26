#include "Renderer.h"
#include "GraphicsPlatformInterface.h"
#include "..\Graphics\Direct3D12\D3D12Interface.h"

namespace Havana::Graphics
{
	namespace
	{
		PlatformInterface gfx{};

		bool SetGraphicsPlatform(GraphicsPlatform platform)
		{
			switch (platform)
			{
			case GraphicsPlatform::Direct3D12:
				D3D12::GetPlatformInterface(gfx);
				break;
			default:
				return false;
			}
			return true;
		}

	} // anonymous namespace

	bool Initialize(GraphicsPlatform platform)
	{
		return SetGraphicsPlatform(platform) && gfx.Initialize();
	}

	void Shutdown()
	{
		gfx.Shutdown();
	}

	void Render()
	{
		gfx.Render();
	}

	Surface CreateSurface(Platform::Window window)
	{
		return gfx.surface.Create(window);
	}

	void RemoveSurface(surface_id id)
	{
		assert(Id::IsValid(id));
		gfx.surface.Remove(id);
	}

	void Surface::Resize(u32 width, u32 height) const
	{
		assert(IsValid());
		return gfx.surface.Resize(m_id, width, height);
	}

	u32 Surface::Width() const
	{
		assert(IsValid());
		return gfx.surface.Width(m_id);
	}

	u32 Surface::Height() const
	{
		assert(IsValid());
		return gfx.surface.Height(m_id);
	}

	void Surface::Render() const
	{
		assert(IsValid());
		gfx.surface.Render(m_id);
	}
}