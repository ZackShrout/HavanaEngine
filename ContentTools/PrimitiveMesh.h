#pragma once

#include "ToolsCommon.h"

namespace Havana::Tools
{
	enum PrimitiveMeshType : u32
	{
		Plane,
		Cube,
		UVSphere,
		ICOSphere,
		Cylinder,
		Capsule,

		Count
	};

	struct PrimitiveInitInfo
	{
		PrimitiveMeshType	type;
		u32					segments[3]{ 1, 1, 1 };
		Math::Vec3			size{ 1,1,1 };
		u32					LoD{ 0 };
	};
}