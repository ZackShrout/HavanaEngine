#pragma once

struct ShaderType
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

struct ShaderFileInfo
{
	const char*			fileName;
	const char*			function;
	ShaderType::type	type;
};

bool CompileShaders();