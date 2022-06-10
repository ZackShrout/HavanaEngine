#pragma once
#include "CommonHeaders.h"

namespace Havana::Content
{
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
}