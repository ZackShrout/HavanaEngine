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

bool compile_shaders();