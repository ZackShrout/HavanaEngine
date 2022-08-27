#pragma once
#include "D3D12CommonHeaders.h"

namespace havana::Graphics::D3D12::Content
{
	namespace Submesh
	{
		Id::id_type Add(const u8* &data);
		void Remove(Id::id_type id);
	} // namespace Submesh
}