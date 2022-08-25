#ifdef _WIN64
#include "..\packages\DirectXShaderCompiler\inc\d3d12shader.h"
#include "..\packages\DirectXShaderCompiler\inc\dxcapi.h"
#include "Graphics\Direct3D12\D3D12Core.h"
#include "Graphics\Direct3D12\D3D12Shaders.h"

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
	constexpr const char* shadersSourcePath{ "../../Engine/Graphics/Direct3D12/Shaders/" };

	struct EngineShaderInfo
	{
		EngineShader::ID	id;
		ShaderFileInfo		info;
	};
	
	constexpr EngineShaderInfo engineShaderFiles[]
	{
		EngineShader::fullscreenTriangleVS, {"FullScreenTriangle.hlsl", "FullScreenTriangleVS", ShaderType::vertex},
		EngineShader::fillColorPS, {"FillColor.hlsl", "FillColorPS", ShaderType::pixel},
		EngineShader::postProcessPS, {"PostProcess.hlsl", "PostProcessPS", ShaderType::pixel},
	};

	static_assert(_countof(engineShaderFiles) == EngineShader::count);

	struct dxc_compiled_shader
	{
		ComPtr<IDxcBlob>		byte_code;
		ComPtr<IDxcBlobUtf8>	disassembly;
		DxcShaderHash			hash;
	};

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

		dxc_compiled_shader Compile(ShaderFileInfo info, std::filesystem::path fullPath)
		{
			assert(m_compiler && m_utils && m_includeHandler);
			HRESULT hr{ S_OK };

			// Load the source file using Utils interface
			ComPtr<IDxcBlobEncoding> sourceBlob{ nullptr };
			DXCall(hr = m_utils->LoadFile(fullPath.c_str(), nullptr, &sourceBlob));
			if (FAILED(hr)) return {};
			assert(sourceBlob && sourceBlob->GetBufferSize());

			std::wstring file{ ToWString(info.fileName) };
			std::wstring func{ ToWString(info.function) };
			std::wstring prof{ ToWString(m_profileStrings[(u32)info.type]) };
			std::wstring inc{ ToWString(shadersSourcePath) };

			LPCWSTR args[]
			{
				file.c_str(),											// optional shader source file name for error reporting
				L"-E", func.c_str(),									// entry function
				L"-T", prof.c_str(),									// target profile
				L"-I", inc.c_str(),										// include path
				L"-enable-16bit-types",
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
			OutputDebugStringA(info.fileName);
			OutputDebugStringA(" : ");
			OutputDebugStringA(info.function);
			OutputDebugStringA("\n");

			return Compile(sourceBlob.Get(), args, _countof(args));
		}

		dxc_compiled_shader Compile(IDxcBlobEncoding* sourceBlob, LPCWSTR* args, u32 numArgs)
		{
			DxcBuffer buffer{};
			buffer.Encoding = DXC_CP_ACP;					// auto detect format
			buffer.Ptr = sourceBlob->GetBufferPointer();
			buffer.Size = sourceBlob->GetBufferSize();

			HRESULT hr{ S_OK };
			ComPtr<IDxcResult> results{ nullptr };
			DXCall(hr = m_compiler->Compile(&buffer, args, numArgs, m_includeHandler.Get(), IID_PPV_ARGS(&results)));
			if (FAILED(hr)) return {};

			ComPtr<IDxcBlobUtf8> errors{ nullptr };
			DXCall(hr = results->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr));
			if (FAILED(hr)) return {};

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
			if (FAILED(hr) || FAILED(status)) return {};

			ComPtr<IDxcBlob> hash{ nullptr };
			DXCall(hr = results->GetOutput(DXC_OUT_SHADER_HASH, IID_PPV_ARGS(&hash), nullptr));
			if (FAILED(hr)) return {};
			DxcShaderHash* const hash_buffer{ (DxcShaderHash* const)hash->GetBufferPointer() };
			// Different source code could result in the same byte code, so we only care about byte code hash
			assert(!(hash_buffer->Flags & DXC_HASHFLAG_INCLUDES_SOURCE));
			OutputDebugStringA("Shader hash: ");
			for (u32 i{ 0 }; i < _countof(hash_buffer->HashDigest); ++i)
			{
				char hash_bytes[3]{}; // 2 chars for hex value plus termination 0.
				sprintf_s(hash_bytes, "%02x", (u32)hash_buffer->HashDigest[i]);
				OutputDebugStringA(hash_bytes);
				OutputDebugStringA(" ");
			}
			OutputDebugStringA("\n");

			ComPtr<IDxcBlob> shader{ nullptr };
			DXCall(hr = results->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shader), nullptr));
			if (FAILED(hr)) return {};
			buffer.Ptr = shader->GetBufferPointer();
			buffer.Size = shader->GetBufferSize();

			ComPtr<IDxcResult> disasm_results{ nullptr };
			DXCall(hr = m_compiler->Disassemble(&buffer, IID_PPV_ARGS(&disasm_results)));

			ComPtr<IDxcBlobUtf8> disassembly{ nullptr };
			DXCall(hr = disasm_results->GetOutput(DXC_OUT_DISASSEMBLY, IID_PPV_ARGS(&disassembly), nullptr));

			dxc_compiled_shader result{ shader.Detach(), disassembly.Detach() };
			memcpy(&result.hash.HashDigest[0], &hash_buffer->HashDigest[0], _countof(hash_buffer->HashDigest));

			return result;
		}
	private:
		// NOTE: Shader Model 6.x can also be used (AS and MS are only supported from SM6.5 and up)
		constexpr static const char* m_profileStrings[]{ "vs_6_6", "hs_6_6", "ds_6_6", "gs_6_6", "ps_6_6", "cs_6_6", "as_6_6", "ms_6_6" };
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

		std::filesystem::path fullPath{};

		// Check if either of the engine shader source files are newer than the compiled shader file,
		// in which case, we need to recompile
		for (u32 i{ 0 }; i < EngineShader::count; i++)
		{
			auto& file = engineShaderFiles[i];
			fullPath = shadersSourcePath;
			fullPath += file.info.fileName;
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
	bool SaveCompiledShaders(Utils::vector<dxc_compiled_shader>& shaders)
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
			const D3D12_SHADER_BYTECODE byteCode{ shader.byte_code->GetBufferPointer(), shader.byte_code->GetBufferSize() };
			file.write((char*)&byteCode.BytecodeLength, sizeof(byteCode.BytecodeLength));
			file.write((char*)&shader.hash.HashDigest[0], sizeof(shader.hash.HashDigest));
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

	ShaderCompiler compiler{};
	Utils::vector<dxc_compiled_shader> shaders;
	std::filesystem::path fullPath{};

	// Compile shaders put all shaders together in a buffer in the same order of compilation
	for (u32 i{ 0 }; i < EngineShader::count; i++)
	{
		auto& file = engineShaderFiles[i];

		fullPath = shadersSourcePath;
		fullPath += file.info.fileName;
		if (!std::filesystem::exists(fullPath)) return false;
		dxc_compiled_shader compiledShader{ compiler.Compile(file.info, fullPath) };
		if (compiledShader.byte_code && compiledShader.byte_code->GetBufferPointer() && compiledShader.byte_code->GetBufferSize())
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