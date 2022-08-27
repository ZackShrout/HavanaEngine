#pragma once

namespace havana::Graphics
{
	struct PlatformInterface;

	namespace Vulkan
	{
		void GetPlatformInterface(PlatformInterface& platformInterface);
	} // Vulkan namespace
}