#pragma once

namespace Havana::Graphics
{
	struct PlatformInterface;

	namespace D3D12
	{
		void GetPlatformInterface(PlatformInterface& platformInterface);
	} // D3D12 namespace
}