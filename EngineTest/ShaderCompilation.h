#pragma once

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

struct ShaderFileInfo
{
	const char*			fileName;
	const char*			function;
	ShaderType::Type	type;
};

bool CompileShaders();