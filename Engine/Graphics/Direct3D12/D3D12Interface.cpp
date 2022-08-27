#include "CommonHeaders.h"
#include "D3D12Interface.h"
#include "D3D12Core.h"
#include "D3D12Content.h"
#include "D3D12Camera.h"
#include "Graphics/GraphicsPlatformInterface.h"

namespace havana::Graphics
{
	namespace D3D12
	{
		void GetPlatformInterface(PlatformInterface& pi)
		{
			pi.Initialize = Core::Initialize;
			pi.Shutdown = Core::Shutdown;

			pi.Surface.Create = Core::CreateSurface;
			pi.Surface.Remove = Core::RemoveSurface;
			pi.Surface.Resize = Core::ResizeSurface;
			pi.Surface.Width = Core::SurfaceWidth;
			pi.Surface.Height = Core::SurfaceHeight;
			pi.Surface.Render = Core::RenderSurface;

			pi.Camera.Create = Camera::Create;
			pi.Camera.Remove = Camera::Remove;
			pi.Camera.SetParamter = Camera::SetParamter;
			pi.Camera.GetParamter = Camera::GetParamter;

			pi.Resources.AddSubmesh = Content::Submesh::Add;
			pi.Resources.RemoveSubmesh = Content::Submesh::Remove;

			pi.platform = GraphicsPlatform::Direct3D12;
		}
	} // D3D12 namespace
}