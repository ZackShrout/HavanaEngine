#pragma once
#include "CommonHeaders.h"

namespace havana::Content
{
	struct AssetType
	{
		enum type : u32
		{
			Unknown = 0,
			Animation,
			Audio,
			Material,
			Mesh,
			Skeleton,
			Texture,

			count
		};
	};
	
	struct PrimitiveTopology
	{
		enum type: u32
		{
			PointList = 1,
			LineList,
			LineStrip,
			TriangleList,
			TriangleStrip,

			count
		};
	};

	Id::id_type CreateResource(const void* const data, AssetType::type type);
	void DestroyResource(Id::id_type id, AssetType::type type);
}