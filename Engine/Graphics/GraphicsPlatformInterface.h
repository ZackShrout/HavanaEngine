#pragma once
#include "../Common/CommonHeaders.h"
#include "Renderer.h"
#include "../Platforms/Window.h"

namespace Havana::Graphics
{
	struct PlatformInterface
	{
		bool(*Initialize)(void);
		void(*Shutdown)(void);

		struct
		{
			Surface(*Create)(Platform::Window);
			void(*Remove)(surface_id);
			void(*Resize)(surface_id, u32, u32);
			u32(*Width)(surface_id);
			u32(*Height)(surface_id);
			void(*Render)(surface_id);
		} Surface;
	};
}