#pragma once
#include "..\Common\CommonHeaders.h"
#include "..\Platforms\Platform.h"

namespace Havana::Graphics
{
	class Surface
	{

	};

	struct RenderSurface
	{
		Platform::Window window{};
		Surface surface{};
	};

	enum GraphicsPlatform : u32
	{
		Direct3D12 = 0
	};
	
	bool Initialize(GraphicsPlatform platform);
	void Shutdown();
}