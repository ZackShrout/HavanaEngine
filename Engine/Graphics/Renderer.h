#pragma once
#include "..\Common\CommonHeaders.h"
#include "..\Platforms\Platform.h"

namespace Havana::Graphics
{
	class Surface
	{

	};

	struct RenderSurface
	{
		Platform::Window window{};
		Surface surface{};
	};
}