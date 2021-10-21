#include "D3D12Shaders.h"
#include "Content/ContentLoader.h"

namespace Havana::Graphics::D3D12::Shaders
{
	namespace
	{
		typedef struct CompiledShader
		{
			u64			size;
			const u8*	byteCode;
		} const* compiledShaderPtr;

		// Each element in this array points to an offset within the shaders blob
		compiledShaderPtr engineShaders[EngineShader::count]{};
		
		// This is a chunk of memory that contains all compiled engine shaders
		// The blob is an array of shader byte code, consisting of a u64 size and
		// and array of bytes.
		std::unique_ptr<u8[]> shadersBlob{};
		
		bool LoadEngineShaders()
		{
			assert(!shadersBlob);
			u64 size{ 0 };
			bool result{ Content::LoadEngineShaders(shadersBlob, size) };
			assert(shadersBlob && size);

			u64 offset{ 0 };
			u64 index{ 0 };
			while (offset < size && result)
			{
				assert(index < EngineShader::count);
				compiledShaderPtr& shader{ engineShaders[index] };
				assert(!shader);
				result &= index < EngineShader::count && !shader;
				if (!result) break;

				shader = reinterpret_cast<const compiledShaderPtr>(&shadersBlob[offset]);
				offset += sizeof(u64) + shader->size;
				index++;
			}
			assert(offset == size && index == EngineShader::count);

			return result;
		}

	} // anonymous namespace

	bool Initialize()
	{
		return LoadEngineShaders();
	}

	void Shutdown()
	{
		for (u32 i{ 0 }; i < EngineShader::count; i++)
		{
			engineShaders[i] = {};
		}
		shadersBlob.reset();
	}

	D3D12_SHADER_BYTECODE GetEngineShader(EngineShader::ID id)
	{
		assert(id < EngineShader::count);
		const compiledShaderPtr shader{ engineShaders[id] };
		assert(shader && shader->size);

		return { &shader->byteCode, shader->size };
	}
}

