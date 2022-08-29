#pragma once
#include "D3D12CommonHeaders.h"

namespace havana::graphics::d3d12::content
{
	namespace Submesh
	{
		id::id_type Add(const u8* &data);
		void Remove(id::id_type id);
	} // namespace Submesh
}