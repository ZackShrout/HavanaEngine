#pragma once
#include "D3D12CommonHeaders.h"

namespace havana::graphics::d3d12::shaders
{
	struct engine_shader
	{
		enum ID : u32
		{
			fullscreen_triangle_vs = 0,
			fill_color_ps = 1,
			postProcessPS = 2,

			count
		};
	};

	bool initialize();
	void shutdown();

	D3D12_SHADER_BYTECODE get_engine_shader(engine_shader::ID id);
}