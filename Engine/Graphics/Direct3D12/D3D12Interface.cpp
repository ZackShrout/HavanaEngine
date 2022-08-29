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
			pi.initialize = Core::initialize;
			pi.shutdown = Core::shutdown;

			pi.surface.create = Core::create_surface;
			pi.surface.remove = Core::remove_surface;
			pi.surface.resize = Core::ResizeSurface;
			pi.surface.width = Core::SurfaceWidth;
			pi.surface.height = Core::SurfaceHeight;
			pi.surface.render = Core::render_surface;

			pi.camera.create = camera::Create;
			pi.camera.remove = camera::Remove;
			pi.camera.set_paramter = camera::SetParamter;
			pi.camera.get_paramter = camera::GetParamter;

			pi.resources.add_submesh = content::Submesh::Add;
			pi.resources.remove_submesh = content::Submesh::Remove;

			pi.platform = graphics_platform::direct3d12;
		}
	} // d3d12 namespace
}