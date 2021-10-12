#pragma once
#include "VulkanCommonHeaders.h"

namespace Havana::Graphics::Vulkan::Core
{
	bool Initialize();
	void Shutdown();

	u32 CurrentFrameIndex();

	Surface CreateSurface(Platform::Window window);
	void RemoveSurface(surface_id id);
	void ResizeSurface(surface_id id, u32, u32);
	u32 SurfaceWidth(surface_id id);
	u32 SurfaceHeight(surface_id id);
	void RenderSurface(surface_id id);
}