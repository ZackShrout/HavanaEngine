#include "D3D12GPass.h"
#include "D3D12Core.h"
#include "D3D12Shaders.h"

namespace havana::Graphics::D3D12::GPass
{
	namespace
	{
		struct GPassRootParamIndices
		{
			enum : u32
			{
				rootConstants,

				count
			};
		};

		constexpr DXGI_FORMAT			mainBufferFormat{ DXGI_FORMAT_R16G16B16A16_FLOAT };
		constexpr DXGI_FORMAT			depthBufferFormat{ DXGI_FORMAT_D32_FLOAT };
		constexpr math::Vec2u32			initialDimensions{ 100, 100 };
		
		D3D12RenderTexture				gpassMainBuffer{};
		D3D12DepthBuffer				gpassDepthBuffer{};
		math::Vec2u32					dimensions{ initialDimensions };
		D3D12_RESOURCE_BARRIER_FLAGS	flags{};

		ID3D12RootSignature*			gpassRootSig{ nullptr };
		ID3D12PipelineState*			gpassPSO{ nullptr };

#if _DEBUG
		constexpr f32					clearValue[4]{ 0.5f, 0.5f, 0.5f, 1.0f };
#else
		constexpr f32					clearValue[4]{ };
#endif

		bool CreateBuffers(math::Vec2u32 size)
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

			flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

			return gpassMainBuffer.Resource() && gpassDepthBuffer.Resource();
		}
	} // anonymous namespace

	bool CreateGPassPSOandRootSignature()
	{
		assert(!gpassRootSig && !gpassPSO);

		// Create GPass root signature
		using idx = GPassRootParamIndices;
		D3DX::D3D12_Root_Parameter paramters[idx::count]{};
		paramters[0].AsConstants(3, D3D12_SHADER_VISIBILITY_PIXEL, 1);
		const D3DX::D3D12_Root_Signature_Desc rootSignature{ &paramters[0], idx::count };
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
		NAME_D3D12_OBJECT(gpassRootSig, L"GPass Pipeline State Object");

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

	const D3D12RenderTexture& MainBuffer()
	{
		return gpassMainBuffer;
	}

	const D3D12DepthBuffer& DepthBuffer()
	{
		return gpassDepthBuffer;
	}

	void SetSize(math::Vec2u32 size)
	{
		math::Vec2u32& d{ dimensions };
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

		static u32 frame{ 0 };
		struct
		{
			f32 width;
			f32 height;
			u32 frame;
		} constants{ (f32)info.surfaceWidth, (f32)info.surfaceHeight, ++frame };

		using idx = GPassRootParamIndices;
		cmdList->SetGraphicsRoot32BitConstants(idx::rootConstants, 3, &constants, 0);

		cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		cmdList->DrawInstanced(3, 1, 0, 0);
	}

	void AddTransitionsForDepthPrepass(D3DX::ResourceBarrier& barriers)
	{
		barriers.Add(gpassMainBuffer.Resource(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY);
		barriers.Add(gpassDepthBuffer.Resource(),
			D3D12_RESOURCE_STATE_DEPTH_READ | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_DEPTH_WRITE, flags);

		flags = D3D12_RESOURCE_BARRIER_FLAG_END_ONLY;
	}

	void AddTransitionsForGPass(D3DX::ResourceBarrier& barriers)
	{
		barriers.Add(gpassMainBuffer.Resource(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_BARRIER_FLAG_END_ONLY);
		barriers.Add(gpassDepthBuffer.Resource(),
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			D3D12_RESOURCE_STATE_DEPTH_READ | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	}

	void AddTransitionsForPostProcess(D3DX::ResourceBarrier& barriers)
	{
		barriers.Add(gpassMainBuffer.Resource(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		barriers.Add(gpassDepthBuffer.Resource(),
			D3D12_RESOURCE_STATE_DEPTH_READ | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY);
	}

	void SetRenderTargetsForDepthPrepass(id3d12GraphicsCommandList* cmdList)
	{
		const D3D12_CPU_DESCRIPTOR_HANDLE dsv{ gpassDepthBuffer.DSV() };
		cmdList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 0.0f, 0, 0, nullptr);
		cmdList->OMSetRenderTargets(0, nullptr, 0, &dsv);
	}

	void SetRenderTargetsForGPass(id3d12GraphicsCommandList* cmdList)
	{
		const D3D12_CPU_DESCRIPTOR_HANDLE rtv{ gpassMainBuffer.RTV(0) };
		const D3D12_CPU_DESCRIPTOR_HANDLE dsv{ gpassDepthBuffer.DSV() };

		cmdList->ClearRenderTargetView(rtv, clearValue, 0, nullptr);
		cmdList->OMSetRenderTargets(1, &rtv, 0, &dsv);
	}
}