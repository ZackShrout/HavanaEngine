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
}