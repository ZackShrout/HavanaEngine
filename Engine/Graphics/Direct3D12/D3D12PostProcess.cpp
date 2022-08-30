#include "D3D12PostProcess.h"
#include "D3D12Core.h"
#include "D3D12Shaders.h"
#include "D3D12Surface.h"
#include "D3D12GPass.h"

namespace havana::graphics::d3d12::fx
{
	namespace
	{
		struct FXRootParamIndices
		{
			enum : u32
			{
				root_constants,
				descriptorTable,

				count
			};
		};

		ID3D12RootSignature*	fxRootSig{ nullptr };
		ID3D12PipelineState*	fxPSO{ nullptr };

		bool CreateFXPSOAndRootSignature()
		{
			assert(!fxRootSig && !fxPSO);
			
			// Create Post-Process fx root signature
			d3dx::d3d12_descriptor_range range
			{
				D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
				D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND, 0, 0,
				D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE
			};

			using idx = FXRootParamIndices;
			d3dx::d3d12_root_parameter paramters[idx::count]{};
			paramters[idx::root_constants].as_constants(1, D3D12_SHADER_VISIBILITY_PIXEL, 1);
			paramters[idx::descriptorTable].as_descriptor_table(D3D12_SHADER_VISIBILITY_PIXEL, &range, 1);

			const d3dx::d3d12_root_signature_desc rootSignature{ &paramters[0], _countof(paramters) };
			fxRootSig = rootSignature.create();
			assert(fxRootSig);
			NAME_D3D12_OBJECT(fxRootSig, L"Post-Process FX Root Signature");

			// Create Post-Process fx Pipeline State Object
			struct
			{
				d3dx::d3d12_pipeline_state_subobject_root_signature			rootSignature{ fxRootSig };
				d3dx::d3d12_pipeline_state_subobject_vs						vs{ shaders::get_engine_shader(shaders::engine_shader::fullscreen_triangle_vs) };
				d3dx::d3d12_pipeline_state_subobject_ps						ps{ shaders::get_engine_shader(shaders::engine_shader::postProcessPS) };
				d3dx::d3d12_pipeline_state_subobject_primitive_topology		primitive_topology{ D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE };
				d3dx::d3d12_pipeline_state_subobject_render_target_formats	renderTargetFormats;
				d3dx::d3d12_pipeline_state_subobject_rasterizer				rasterizer{ d3dx::rasterizer_state.no_cull };
			} stream;

			D3D12_RT_FORMAT_ARRAY rtfArray{};
			rtfArray.NumRenderTargets = 1;
			rtfArray.RTFormats[0] = d3d12_surface::defaultBackBufferFormat;

			stream.renderTargetFormats = rtfArray;

			fxPSO = d3dx::create_pipeline_state(&stream, sizeof(stream));
			NAME_D3D12_OBJECT(fxRootSig, L"Post-Process FX Pipeline State Object");

			return fxRootSig && fxPSO;
		}
	} //anonymous namespace

	bool initialize()
	{
		return CreateFXPSOAndRootSignature();
	}

	void shutdown()
	{
		core::release(fxRootSig);
		core::release(fxPSO);
	}

	void post_process(ID3D12GraphicsCommandList* cmdList, D3D12_CPU_DESCRIPTOR_HANDLE targetRTV)
	{
		cmdList->SetGraphicsRootSignature(fxRootSig);
		cmdList->SetPipelineState(fxPSO);

		using idx = FXRootParamIndices;
		cmdList->SetGraphicsRoot32BitConstant(idx::root_constants, gpass::main_buffer().SRV().index, 0);
		cmdList->SetGraphicsRootDescriptorTable(idx::descriptorTable, core::srv_heap().GpuStart());
		cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		// NOTE: we don't need to clear the render target, because each pixel will
		//		 be overwritten by pixels from the gpass main buffer.
		//		 We also don't need a depth buffer.
		cmdList->OMSetRenderTargets(1, &targetRTV, 1, nullptr);
		cmdList->DrawInstanced(3, 1, 0, 0);
	}
}
