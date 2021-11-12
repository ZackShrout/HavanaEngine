#include "D3D12GPass.h"
#include "D3D12Core.h"
#include "D3D12Shaders.h"

namespace Havana::Graphics::D3D12::GPass
{
	namespace
	{
		constexpr DXGI_FORMAT			mainBufferFormat{ DXGI_FORMAT_R16G16B16A16_FLOAT };
		constexpr DXGI_FORMAT			depthBufferFormat{ DXGI_FORMAT_D32_FLOAT };
		constexpr Math::Vec2u32			initialDimensions{ 100, 100 };
		
		D3D12RenderTexture				gpassMainBuffer{};
		D3D12DepthBuffer				gpassDepthBuffer{};
		Math::Vec2u32					dimensions{ initialDimensions };

		ID3D12RootSignature*			gpassRootSig{ nullptr };
		ID3D12PipelineState*			gpassPSO{ nullptr };

#if _DEBUG
		constexpr f32					clearValue[4]{ 0.5f, 0.5f, 0.5f, 1.0f };
#else
		constexpr f32					clearValue[4]{ };
#endif

		bool CreateBuffers(Math::Vec2u32 size)
		{
			assert(size.x && size.y);
			gpassMainBuffer.Release();
			gpassDepthBuffer.Release();

			D3D12_RESOURCE_DESC desc{};
			desc.Alignment = 0; // NOTE: 0 is the same as 64KB (or 4MB for MSAA)
			desc.DepthOrArraySize = 1;
			desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
			desc.Format = mainBufferFormat;
			desc.Height = size.y;
			desc.Width = size.x;
			desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
			desc.MipLevels = 0; // Make space for all mip levels
			desc.SampleDesc = { 1, 0 };

			// Create the main buffer
			{
				D3D12TextureInitInfo info{};
				info.desc = &desc;
				info.initialState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
				info.clearValue.Format = desc.Format;
				memcpy(&info.clearValue.Color, &clearValue[0], sizeof(clearValue));
				
				gpassMainBuffer = D3D12RenderTexture{ info };
			}

			desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
			desc.Format = depthBufferFormat;
			desc.MipLevels = 1;
			
			// Create the depth buffer
			{
				D3D12TextureInitInfo info{};
				info.desc = &desc;
				info.initialState = D3D12_RESOURCE_STATE_DEPTH_READ | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
				info.clearValue.Format = desc.Format;
				info.clearValue.DepthStencil.Depth = 0.0f;
				info.clearValue.DepthStencil.Stencil = 0;

				gpassDepthBuffer = D3D12DepthBuffer{ info };
			}

			NAME_D3D12_OBJECT(gpassMainBuffer.Resource(), L"GPass Main Buffer");
			NAME_D3D12_OBJECT(gpassDepthBuffer.Resource(), L"GPass Depth Buffer");

			return gpassMainBuffer.Resource() && gpassDepthBuffer.Resource();
		}
	} // anonymous namespace

	bool CreateGPassPSOandRootSignature()
	{
		assert(!gpassRootSig && !gpassPSO);

		// Create GPass root signature
		D3DX::D3D12_Root_Parameter paramters[1]{};
		paramters[0].AsConstants(1, D3D12_SHADER_VISIBILITY_PIXEL, 1);
		const D3DX::D3D12_Root_Signature_Desc rootSignature{ &paramters[0], _countof(paramters) };
		gpassRootSig = rootSignature.Create();
		assert(gpassRootSig);
		NAME_D3D12_OBJECT(gpassRootSig, L"GPass Root Signature");

		// Create GPass Pipeline State Object
		struct
		{
			D3DX::D3D12_Pipeline_State_Subobject_rootSignature			rootSignature{ gpassRootSig };
			D3DX::D3D12_Pipeline_State_Subobject_vs						vs{ Shaders::GetEngineShader(Shaders::EngineShader::fullscreenTriangleVS) };
			D3DX::D3D12_Pipeline_State_Subobject_ps						ps{ Shaders::GetEngineShader(Shaders::EngineShader::fillColorPS) };
			D3DX::D3D12_Pipeline_State_Subobject_primitiveTopology		primitiveTopology{ D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE };
			D3DX::D3D12_Pipeline_State_Subobject_renderTargetFormats	renderTargetFormats;
			D3DX::D3D12_Pipeline_State_Subobject_depthStencilFormat		depthStencilFormat{ depthBufferFormat };
			D3DX::D3D12_Pipeline_State_Subobject_rasterizer				rasterizer{ D3DX::rasterizerState.noCull };
			D3DX::D3D12_Pipeline_State_Subobject_depthStencil1			depth{ D3DX::depthState.disabled };
		} stream;

		D3D12_RT_FORMAT_ARRAY rtfArray{};
		rtfArray.NumRenderTargets = 1;
		rtfArray.RTFormats[0] = mainBufferFormat;

		stream.renderTargetFormats = rtfArray;

		gpassPSO = D3DX::CreatePipelineState(&stream, sizeof(stream));
		NAME_D3D12_OBJECT(gpassRootSig, L"GPass PSO");

		return gpassRootSig && gpassPSO;
	}

	bool Initialize()
	{
		return CreateBuffers(initialDimensions) && CreateGPassPSOandRootSignature();
	}

	void Shutdown()
	{
		gpassMainBuffer.Release();
		gpassDepthBuffer.Release();
		dimensions = initialDimensions;

		Core::Release(gpassRootSig);
		Core::Release(gpassPSO);
	}

	void SetSize(Math::Vec2u32 size)
	{
		Math::Vec2u32& d{ dimensions };
		if (size.x > d.x || size.y > d.y)
		{
			d = { std::max(size.x, d.x), std::max(size.y, d.y) };
			CreateBuffers(d);
		}
	}
	void DepthPrepass(ID3D12GraphicsCommandList* cmdList, const D3D12FrameInfo& info)
	{

	}

	void Render(ID3D12GraphicsCommandList* cmdList, const D3D12FrameInfo& info)
	{
		cmdList->SetGraphicsRootSignature(gpassRootSig);
		cmdList->SetPipelineState(gpassPSO);
	}
}