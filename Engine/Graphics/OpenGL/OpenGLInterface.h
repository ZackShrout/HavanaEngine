#pragma once

namespace havana::Graphics
{
	struct PlatformInterface;

	namespace OpenGL
	{
		void GetPlatformInterface(PlatformInterface& platformInterface);
	} // OpenGL namespace
}