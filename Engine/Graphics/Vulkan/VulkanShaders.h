#pragma once
#include "VulkanCommonHeaders.h"

namespace Havana::Graphics::Vulkan::Shaders
{
	struct ShaderType
	{
		enum Type : u32
		{
			vertex = 0,
			hull,
			domain,
			geometry,
			pixel,
			compute,
			amplification,
			mesh,

			count
		};
	};

	struct EngineShader
	{
		enum ID : u32
		{
			fullscreenTriangleVS = 0,

			count
		};
	};

	bool Initialize();
	void Shutdown();

	const u8* const* GetEngineShader(EngineShader::ID id);
}