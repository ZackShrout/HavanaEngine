#pragma once
#include "D3D12CommonHeaders.h"

namespace Havana::Graphics::D3D12::Shaders
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

	D3D12_SHADER_BYTECODE GetEngineShader(EngineShader::ID id);
}