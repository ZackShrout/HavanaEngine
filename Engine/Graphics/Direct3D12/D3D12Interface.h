#pragma once

namespace havana::graphics
{
	struct platform_interface;

	namespace d3d12
	{
		void get_platform_interface(platform_interface& pi);
	} // d3d12 namespace
}