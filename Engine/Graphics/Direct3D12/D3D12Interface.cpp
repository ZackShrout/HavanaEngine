#include "../Common/CommonHeaders.h"
#include "../Graphics/GraphicsPlatformInterface.h"
#include "D3D12Interface.h"
#include "D3D12Core.h"

namespace Havana::Graphics
{
	namespace D3D12
	{
		void GetPlatformInterface(PlatformInterface& platformInterface)
		{
			platformInterface.Initialize = Core::Initialize;
			platformInterface.Shutdown = Core::Shutdown;

			platformInterface.Surface.Create = Core::CreateSurface;
			platformInterface.Surface.Remove = Core::RemoveSurface;
			platformInterface.Surface.Resize = Core::ResizeSurface;
			platformInterface.Surface.Width = Core::SurfaceWidth;
			platformInterface.Surface.Height = Core::SurfaceHeight;
			platformInterface.Surface.Render = Core::RenderSurface;

			platformInterface.platform = GraphicsPlatform::Direct3D12;
		}
	} // D3D12 namespace
}