#pragma once

namespace Havana::Graphics
{
	struct PlatformInterface;

	namespace Vulkan
	{
		void GetPlatformInterface(PlatformInterface& platformInterface);
	} // Vulkan namespace
}