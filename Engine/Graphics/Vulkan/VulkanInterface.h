#pragma once

namespace havana::graphics
{
	struct platform_interface;

	namespace vulkan
	{
		void get_platform_interface(platform_interface& pi);
	}
}