#include "../Common/CommonHeaders.h"
#include "../Graphics/GraphicsPlatformInterface.h"
#include "VulkanInterface.h"
#include "VulkanCore.h"

namespace Havana::Graphics
{
	namespace Vulkan
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

			platformInterface.platform = GraphicsPlatform::VulkanAPI;
		}
	} // Vulkan namespace
}