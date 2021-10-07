#pragma once

namespace Havana::Graphics
{
	struct PlatformInterface;

	namespace OpenGL
	{
		void GetPlatformInterface(PlatformInterface& platformInterface);
	} // OpenGL namespace
}