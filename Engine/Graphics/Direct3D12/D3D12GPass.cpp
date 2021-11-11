#include "D3D12GPass.h"
#include "D3D12Core.h"

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

	bool Initialize()
	{
		return CreateBuffers(initialDimensions);
	}

	void Shutdown()
	{
		gpassMainBuffer.Release();
		gpassDepthBuffer.Release();
		dimensions = initialDimensions;
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
}