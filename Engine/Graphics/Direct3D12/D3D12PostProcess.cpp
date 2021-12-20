#include "D3D12PostProcess.h"
#include "D3D12Core.h"
#include "D3D12Shaders.h"
#include "D3D12Surface.h"
#include "D3D12GPass.h"

namespace Havana::Graphics::D3D12::FX
{
	namespace
	{
		struct FXRootParamIndices
		{
			enum : u32
			{
				rootConstants,
				descriptorTable,

				count
			};
		};

		ID3D12RootSignature*	fxRootSig{ nullptr };
		ID3D12PipelineState*	fxPSO{ nullptr };

		bool CreateFXPSOAndRootSignature()
		{
			assert(!fxRootSig && !fxPSO);
			
			// Create Post-Process FX root signature
			D3DX::D3D12_Descriptor_Range range
			{
				D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
				D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND, 0, 0,
				D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE
			};

			using idx = FXRootParamIndices;
			D3DX::D3D12_Root_Parameter paramters[idx::count]{};
			paramters[idx::rootConstants].AsConstants(1, D3D12_SHADER_VISIBILITY_PIXEL, 1);
			paramters[idx::descriptorTable].AsDescriptorTable(D3D12_SHADER_VISIBILITY_PIXEL, &range, 1);

			const D3DX::D3D12_Root_Signature_Desc rootSignature{ &paramters[0], _countof(paramters) };
			fxRootSig = rootSignature.Create();
			assert(fxRootSig);
			NAME_D3D12_OBJECT(fxRootSig, L"Post-Process FX Root Signature");

			// Create Post-Process FX Pipeline State Object
			struct
			{
				D3DX::D3D12_Pipeline_State_Subobject_rootSignature			rootSignature{ fxRootSig };
				D3DX::D3D12_Pipeline_State_Subobject_vs						vs{ Shaders::GetEngineShader(Shaders::EngineShader::fullscreenTriangleVS) };
				D3DX::D3D12_Pipeline_State_Subobject_ps						ps{ Shaders::GetEngineShader(Shaders::EngineShader::postProcessPS) };
				D3DX::D3D12_Pipeline_State_Subobject_primitiveTopology		primitiveTopology{ D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE };
				D3DX::D3D12_Pipeline_State_Subobject_renderTargetFormats	renderTargetFormats;
				D3DX::D3D12_Pipeline_State_Subobject_rasterizer				rasterizer{ D3DX::rasterizerState.noCull };
			} stream;

			D3D12_RT_FORMAT_ARRAY rtfArray{};
			rtfArray.NumRenderTargets = 1;
			rtfArray.RTFormats[0] = D3D12Surface::defaultBackBufferFormat;

			stream.renderTargetFormats = rtfArray;

			fxPSO = D3DX::CreatePipelineState(&stream, sizeof(stream));
			NAME_D3D12_OBJECT(fxRootSig, L"Post-Process FX Pipeline State Object");

			return fxRootSig && fxPSO;
		}
	} //anonymous namespace

	bool Initialize()
	{
		return CreateFXPSOAndRootSignature();
	}

	void Shutdown()
	{
		Core::Release(fxRootSig);
		Core::Release(fxPSO);
	}

	void PostProcess(ID3D12GraphicsCommandList* cmdList, D3D12_CPU_DESCRIPTOR_HANDLE targetRTV)
	{
		cmdList->SetGraphicsRootSignature(fxRootSig);
		cmdList->SetPipelineState(fxPSO);

		using idx = FXRootParamIndices;
		cmdList->SetGraphicsRoot32BitConstant(idx::rootConstants, GPass::MainBuffer().SRV().index, 0);
		cmdList->SetGraphicsRootDescriptorTable(idx::descriptorTable, Core::SRVHeap().GpuStart());
		cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		// NOTE: we don't need to clear the render target, because each pixel will
		//		 be overwritten by pixels from the gpass main buffer.
		//		 We also don't need a depth buffer.
		cmdList->OMSetRenderTargets(1, &targetRTV, 1, nullptr);
		cmdList->DrawInstanced(3, 1, 0, 0);
	}
}
