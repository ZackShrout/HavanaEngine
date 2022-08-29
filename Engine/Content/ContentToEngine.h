#pragma once
#include "CommonHeaders.h"

namespace havana::content
{
	struct asset_type
	{
		enum type : u32
		{
			unknown = 0,
			animation,
			audio,
			material,
			mesh,
			skeleton,
			texture,

			count
		};
	};
	
	struct primitive_topology
	{
		enum type: u32
		{
			point_list = 1,
			line_list,
			line_strip,
			triangle_list,
			triangle_strip,

			count
		};
	};

	id::id_type create_resource(const void* const data, asset_type::type type);
	void destroy_resource(id::id_type id, asset_type::type type);
}