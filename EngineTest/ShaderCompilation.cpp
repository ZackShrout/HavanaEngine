#ifdef _WIN64
#include <d3d12shader.h>
#include <dxcapi.h>
#include "Graphics/Direct3D12/D3D12Core.h"
#include "Graphics/Direct3D12/D3D12Shaders.h"
using namespace Havana::Graphics::D3D12::Shaders;
using namespace Microsoft::WRL;
#elif __linux__

#endif // _WIN64
#include <glslang/Public/ShaderLang.h>
#include "ShaderCompilation.h"
#include "../Graphics/Vulkan/VulkanCore.h"
#include "../Graphics/Vulkan/VulkanShaders.h"

#include <fstream>
#include <filesystem>

using namespace Havana;
using namespace Havana::Graphics::Vulkan::Shaders;
namespace
{
	struct ShaderFileInfo
	{
		const char*			file;
		const char*			function;
		EngineShader::ID	id;
		ShaderType::Type	type;
	};

	constexpr ShaderFileInfo shaderFiles[]
	{
		{"FullScreenTriangle.hlsl", "FullScreenTriangleVS", EngineShader::fullscreenTriangleVS, ShaderType::vertex},
	};

	static_assert(_countof(shaderFiles) == EngineShader::count);

	constexpr const char* shadersSourcePath{ "../../Engine/Graphics/Direct3D12/Shaders/" };

	// Get the path to the compiled shader's binary file
	decltype(auto) GetEngineShadersPath()
	{
		return std::filesystem::absolute(Graphics::GetEngineShadersPath(Graphics::GraphicsPlatform::Direct3D12));
	}

	bool CompiledShadersAreUpToData()
	{
		auto engineShadersPath = GetEngineShadersPath();
		if (!std::filesystem::exists(engineShadersPath)) return false;
		auto shadersCompilationTime = std::filesystem::last_write_time(engineShadersPath);

		std::filesystem::path path{};
		std::filesystem::path fullPath{};

		// Check if either of the engine shader source files are newer than the compiled shader file,
		// in which case, we need to recompile
		for (u32 i{ 0 }; i < EngineShader::count; i++)
		{
			auto& info = shaderFiles[i];
			path = shadersSourcePath;
			path += info.file;
			fullPath = std::filesystem::absolute(path);
			if (!std::filesystem::exists(fullPath)) return false;

			auto shaderFileTime = std::filesystem::last_write_time(fullPath);
			if (shaderFileTime > shadersCompilationTime)
			{
				return false;
			}
		}

		return true;
	}

#ifdef _WIN64
	bool SaveCompiledShaders(Utils::vector<ComPtr<IDxcBlob>> shaders)
	{
		auto engineShadersPath = GetEngineShadersPath();
		std::filesystem::create_directories(engineShadersPath.parent_path());
		std::ofstream file(engineShadersPath, std::ios::out | std::ios::binary);
		
		if (!file || !std::filesystem::exists(engineShadersPath))
		{
			file.close();
			return false;
		}

		for (auto& shader : shaders)
		{
			const D3D12_SHADER_BYTECODE byteCode{ shader->GetBufferPointer(), shader->GetBufferSize() };
			file.write((char*)&byteCode.BytecodeLength, sizeof(byteCode.BytecodeLength));
			file.write((char*)&byteCode.pShaderBytecode, byteCode.BytecodeLength);
		}

		file.close();
		return true;
	}
#elif __linux__
	bool SaveCompiledShaders(Utils::vector<u8*> shaders)
	{
		auto engineShadersPath = GetEngineShadersPath();

		// TODO: implement

		return true;
	}
#endif // _WIN64
} // anonymous namespace

#ifdef _WIN64
bool CompileShaders()
{
	if (CompiledShadersAreUpToData()) return true;

	Utils::vector<ComPtr<IDxcBlob>> shaders;
	std::filesystem::path path{};
	std::filesystem::path fullPath{};
	
	// Compile shaders put all shaders together in a buffer in the same order of compilation
	for (u32 i{ 0 }; i < EngineShader::count; i++)
	{
		auto& info = shaderFiles[i];
		path = shadersSourcePath;
		path += info.file;
		fullPath = std::filesystem::absolute(path);
		if (!std::filesystem::exists(fullPath)) return false;
		ComPtr<IDxcBlob> compiledShader{ /* call compile shader function*/ };
		if (compiledShader->GetBufferPointer() && compiledShader->GetBufferSize() && compiledShader != nullptr)
		{
			shaders.emplace_back(std::move(compiledShader));
		}
		else
		{
			return false;
		}
	}

	return SaveCompiledShaders(shaders);
}
#elif __linux__
bool CompileShaders()
{
	if (CompiledShadersAreUpToData()) return true;

	Utils::vector<u8*> shaders;

	// TODO: implement

	return SaveCompiledShaders(shaders);
}
#endif // _WIN64