#include "..\Common\CommonHeaders.h"
#include "..\Graphics\GraphicsPlatformInterface.h"
#include "D3D12Interface.h"

namespace Havana::Graphics
{
	namespace D3D12
	{
		void GetPlatformInterface(PlatformInterface& platformInterface)
		{
			platformInterface.Initialize = core::Initialize;
			platformInterface.Shutdown = core::Shutdown;
		}
	} // D3D12 namespace
}