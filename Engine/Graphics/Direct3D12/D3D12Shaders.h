#pragma once
#include "D3D12CommonHeaders.h"

namespace Havana::Graphics::D3D12::Shaders
{
	struct EngineShader
	{
		enum ID : u32
		{
			fullscreenTriangleVS = 0,
			fillColorPS = 1,
			postProcessPS = 2,

			count
		};
	};

	bool Initialize();
	void Shutdown();

	D3D12_SHADER_BYTECODE GetEngineShader(EngineShader::ID id);
}