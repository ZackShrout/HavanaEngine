#pragma once
#include "D3D12CommonHeaders.h"

namespace havana::Graphics::D3D12::Content
{
	namespace Submesh
	{
		id::id_type Add(const u8* &data);
		void Remove(id::id_type id);
	} // namespace Submesh
}