#pragma once
#include "..\Common\CommonHeaders.h"
#include "Renderer.h"

namespace Havana::Graphics
{
	struct PlatformInterface
	{
		bool(*Initialize)(void);
		void(*Shutdown)(void);
	};
}