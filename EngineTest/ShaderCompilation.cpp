#ifdef _WIN64
#include "..\packages\DirectXShaderCompiler\inc\d3d12shader.h"
#include "..\packages\DirectXShaderCompiler\inc\dxcapi.h"
#include "Graphics/Direct3D12/D3D12Core.h"
#include "Graphics/Direct3D12/D3D12Shaders.h"

#pragma comment(lib, "../packages/DirectXShaderCompiler/lib/x64/dxcompiler.lib")

using namespace Havana::Graphics::D3D12::Shaders;
using namespace Microsoft::WRL;
#elif __linux__

#include "../Graphics/Vulkan/VulkanShaders.h"
using namespace Havana::Graphics::Vulkan::Shaders;
#endif // _WIN64

#include <glslang/Public/ShaderLang.h>
#include "ShaderCompilation.h"
#include "../Graphics/Vulkan/VulkanCore.h"

#include <fstream>
#include <filesystem>

#define D3D12API 0
#define VULKANAPI 1

using namespace Havana;

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
		{"FillColor.hlsl", "FillColorPS", EngineShader::fillColorPS, ShaderType::pixel},
		{"PostProcess.hlsl", "PostProcessPS", EngineShader::postProcessPS, ShaderType::pixel},
	};

	static_assert(_countof(shaderFiles) == EngineShader::count);

	constexpr const char* shadersSourcePath{ "../../Engine/Graphics/Direct3D12/Shaders/" };

	std::wstring ToWString(const char* c)
	{
		std::string s{ c };
		return { s.begin(), s.end() };
	}

	class ShaderCompiler
	{
	public:
		ShaderCompiler()
		{
			HRESULT hr{ S_OK };
			DXCall(hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&m_compiler)));
			if (FAILED(hr)) return;
			DXCall(hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&m_utils)));
			if (FAILED(hr)) return;
			DXCall(hr = m_utils->CreateDefaultIncludeHandler(&m_includeHandler));
			if (FAILED(hr)) return;
		}
		DISABLE_COPY_AND_MOVE(ShaderCompiler);

		IDxcBlob* Compile(ShaderFileInfo info, std::filesystem::path fullPath)
		{
			assert(m_compiler && m_utils && m_includeHandler);
			HRESULT hr{ S_OK };

			// Load the source file using Utils interface
			ComPtr<IDxcBlobEncoding> sourceBlob{ nullptr };
			DXCall(hr = m_utils->LoadFile(fullPath.c_str(), nullptr, &sourceBlob));
			if (FAILED(hr)) return nullptr;
			assert(sourceBlob && sourceBlob->GetBufferSize());

			std::wstring file{ ToWString(info.file) };
			std::wstring func{ ToWString(info.function) };
			std::wstring prof{ ToWString(m_profileStrings[(u32)info.type]) };
			std::wstring inc{ ToWString(shadersSourcePath) };

			LPCWSTR args[]
			{
				file.c_str(),											// optional shader source file name for error reporting
				L"-E", func.c_str(),									// entry function
				L"-T", prof.c_str(),									// target profile
				L"-I", inc.c_str(),										// include path
				DXC_ARG_ALL_RESOURCES_BOUND,
#if _DEBUG
				DXC_ARG_DEBUG,
				DXC_ARG_SKIP_OPTIMIZATIONS,
#else
				DXC_ARG_OPTIMIZATION_LEVEL3,
#endif // _DEBUG
				DXC_ARG_WARNINGS_ARE_ERRORS,
				L"-Qstrip_reflect",										// strip reflections into a separate blob
				L"-Qstrip_debug",										// strip debug info into a separate blob
			};

			OutputDebugStringA("Compiling ");
			OutputDebugStringA(info.file);

			return Compile(sourceBlob.Get(), args, _countof(args));
		}

		IDxcBlob* Compile(IDxcBlobEncoding* sourceBlob, LPCWSTR* args, u32 numArgs)
		{
			DxcBuffer buffer{};
			buffer.Encoding = DXC_CP_ACP;					// auto detect format
			buffer.Ptr = sourceBlob->GetBufferPointer();
			buffer.Size = sourceBlob->GetBufferSize();

			HRESULT hr{ S_OK };
			ComPtr<IDxcResult> results{ nullptr };
			DXCall(hr = m_compiler->Compile(&buffer, args, numArgs, m_includeHandler.Get(), IID_PPV_ARGS(&results)));
			if (FAILED(hr)) return nullptr;

			ComPtr<IDxcBlobUtf8> errors{ nullptr };
			DXCall(hr = results->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr));
			if (FAILED(hr)) return nullptr;

			if (errors && errors->GetStringLength())
			{
				OutputDebugStringA("\nShader compilation error: \n");
				OutputDebugStringA(errors->GetStringPointer());
			}
			else
			{
				OutputDebugStringA(" [ Succeeded ]");
			}
			OutputDebugStringA("\n");

			HRESULT status{ S_OK };
			DXCall(hr = results->GetStatus(&status));
			if (FAILED(hr) || FAILED(status)) return nullptr;

			ComPtr<IDxcBlob> shader{ nullptr };
			DXCall(hr = results->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shader), nullptr));
			if (FAILED(hr)) return nullptr;

			return shader.Detach();
		}
	private:
		// NOTE: Shader Model 6.x can also be used (AS and MS are only supported from SM6.5 and up)
		constexpr static const char* m_profileStrings[]{ "vs_6_5", "hs_6_5", "ds_6_5", "gs_6_5", "ps_6_5", "cs_6_5", "as_6_5", "ms_6_5" };
		static_assert(_countof(m_profileStrings) == ShaderType::count);

		ComPtr<IDxcCompiler3>		m_compiler{ nullptr };
		ComPtr<IDxcUtils>			m_utils{ nullptr };
		ComPtr<IDxcIncludeHandler>	m_includeHandler{ nullptr };
	};

	// Get the path to the compiled shader's binary file
	decltype(auto) GetEngineShadersPath()
	{
		return std::filesystem::path{ Graphics::GetEngineShadersPath(Graphics::GraphicsPlatform::Direct3D12) };
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
			fullPath = path;
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
			file.write((char*)byteCode.pShaderBytecode, byteCode.BytecodeLength);
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

	ShaderCompiler compiler{};
	// Compile shaders put all shaders together in a buffer in the same order of compilation
	for (u32 i{ 0 }; i < EngineShader::count; i++)
	{
		auto& info = shaderFiles[i];
		path = shadersSourcePath;
		path += info.file;
		fullPath = path;
		if (!std::filesystem::exists(fullPath)) return false;
		ComPtr<IDxcBlob> compiledShader{ compiler.Compile(info, fullPath) };
		if (compiledShader && compiledShader->GetBufferPointer() && compiledShader->GetBufferSize())
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

#if D3D12API
// Compile direct3d12 hlsl into bytecode
#elif VULKANAPI
// Compile vulkan glsl into SPIR-V
#endif