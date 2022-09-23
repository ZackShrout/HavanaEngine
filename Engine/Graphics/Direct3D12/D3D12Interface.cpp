#include "CommonHeaders.h"
#include "D3D12Interface.h"
#include "D3D12Core.h"
#include "D3D12Content.h"
#include "D3D12Camera.h"
#include "Graphics/GraphicsPlatformInterface.h"

namespace havana::graphics
{
	namespace d3d12
	{
		void get_platform_interface(platform_interface& pi)
		{
			pi.initialize = core::initialize;
			pi.shutdown = core::shutdown;

			pi.surface.create = core::create_surface;
			pi.surface.remove = core::remove_surface;
			pi.surface.resize = core::resize_surface;
			pi.surface.width = core::surface_width;
			pi.surface.height = core::surface_height;
			pi.surface.render = core::render_surface;

			pi.camera.create = camera::create;
			pi.camera.remove = camera::remove;
			pi.camera.set_paramter = camera::set_paramter;
			pi.camera.get_paramter = camera::get_paramter;

			pi.resources.add_submesh = content::submesh::add;
			pi.resources.remove_submesh = content::submesh::remove;

			pi.resources.add_material = content::material::add;
			pi.resources.remove_material = content::material::remove;

			pi.platform = graphics_platform::direct3d12;
		}
	} // d3d12 namespace
}