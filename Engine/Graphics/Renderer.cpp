#include "Renderer.h"
#include "GraphicsPlatformInterface.h"

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
				break;
			}
		}

	} // anonymous namespace

	bool Initialize(GraphicsPlatform platform)
	{
		return SetGraphicsPlatform(platform);
	}

	void Shutdown()
	{
		gfx.Shutdown();
	}
}