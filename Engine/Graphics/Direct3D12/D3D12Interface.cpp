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
			pi.initialize = Core::Initialize;
			pi.shutdown = Core::Shutdown;

			pi.surface.create = Core::CreateSurface;
			pi.surface.remove = Core::RemoveSurface;
			pi.surface.resize = Core::ResizeSurface;
			pi.surface.width = Core::SurfaceWidth;
			pi.surface.height = Core::SurfaceHeight;
			pi.surface.render = Core::RenderSurface;

			pi.Camera.create = Camera::Create;
			pi.Camera.remove = Camera::Remove;
			pi.Camera.set_paramter = Camera::SetParamter;
			pi.Camera.get_paramter = Camera::GetParamter;

			pi.Resources.add_submesh = content::Submesh::Add;
			pi.Resources.remove_submesh = content::Submesh::Remove;

			pi.platform = graphics_platform::Direct3D12;
		}
	} // d3d12 namespace
}