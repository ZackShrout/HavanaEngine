#pragma once
#include "CommonHeaders.h"

struct shader_type
{
	enum type : u32
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

struct shader_file_info
{
	const char*			file_name;
	const char*			function;
	shader_type::type	type;
};

std::unique_ptr<u8[]> compile_shader(shader_file_info info, const char* file_path, havana::utl::vector<std::wstring>& extra_args);
bool compile_shaders();