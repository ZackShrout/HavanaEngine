#pragma once
#include "D3D12CommonHeaders.h"

namespace havana::graphics::d3d12::Shaders
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

	bool initialize();
	void shutdown();

	D3D12_SHADER_BYTECODE GetEngineShader(EngineShader::ID id);
}