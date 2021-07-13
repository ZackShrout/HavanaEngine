#include "..\Common\CommonHeaders.h"
#include "..\Graphics\GraphicsPlatformInterface.h"
#include "D3D12Interface.h"
#include "D3DCore.h"

namespace Havana::Graphics
{
	namespace D3D12
	{
		void GetPlatformInterface(PlatformInterface& platformInterface)
		{
			platformInterface.Initialize = Core::Initialize;
			platformInterface.Shutdown = Core::Shutdown;
			platformInterface.Render = Core::Render;
		}
	} // D3D12 namespace
}