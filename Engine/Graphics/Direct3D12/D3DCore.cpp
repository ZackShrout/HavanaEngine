#include "D3DCore.h"

namespace Havana::Graphics::D3D12::Core
{
	namespace
	{
		ID3D12Device8* mainDevice;
	} // anonymous namespace

	bool Initialize()
	{
		// Determine which adapter (i.e. graphics card) to use
		// Determine what the max feature level supported is
		// Create an ID3D12Device (virtual adapter)
	}

	void Shutdown()
	{

	}
}