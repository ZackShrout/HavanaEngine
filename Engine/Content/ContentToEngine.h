#pragma once
#include "CommonHeaders.h"

namespace Havana::Content
{
	struct AssetType
	{
		enum Type : u32
		{
			Unknown = 0,
			Animation,
			Audio,
			Material,
			Mesh,
			Skeleton,
			Texture,

			Count
		};
	};
	
	struct PrimitiveTopology
	{
		enum Type: u32
		{
			PointList = 1,
			LineList,
			LineStrip,
			TriangleList,
			TriangleStrip,

			Count
		};
	};

	Id::id_type CreateResource(const void* const data, AssetType::Type type);
	void DestroyResource(Id::id_type id, AssetType::Type type);
}