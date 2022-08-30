#include "D3D12Surface.h"
#include "D3D12Core.h"

namespace havana::graphics::d3d12
{
	namespace
	{

		constexpr DXGI_FORMAT ToNonSRGB(DXGI_FORMAT format)
		{
			if (format == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB) return DXGI_FORMAT_R8G8B8A8_UNORM;

			return format;
		}
	} // anonymous namespace
	
	// PUBLIC
	void d3d12_surface::CreateSwapChain(IDXGIFactory7* factory, ID3D12CommandQueue* cmdQueue, DXGI_FORMAT format /*= defaultBackBufferFormat*/)
	{
		assert(factory && cmdQueue);
		release();

		if (SUCCEEDED(factory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &m_allowTearing, sizeof(u32))) && m_allowTearing)
		{
			m_presentFlags = DXGI_PRESENT_ALLOW_TEARING;
		}

		m_format = format;

		DXGI_SWAP_CHAIN_DESC1 desc{};
		desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		desc.BufferCount = bufferCount;
		desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		desc.Flags = m_allowTearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
		desc.Format = ToNonSRGB(format);
		desc.Height = m_window.height();
		desc.Width = m_window.width();
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Scaling = DXGI_SCALING_STRETCH;
		desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		desc.Stereo = false;

		IDXGISwapChain1* swapChain;
		HWND hwnd{ (HWND)m_window.handle() };
		DXCall(factory->CreateSwapChainForHwnd(cmdQueue, hwnd, &desc, nullptr, nullptr, &swapChain));
		DXCall(factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER));
		DXCall(swapChain->QueryInterface(IID_PPV_ARGS(&m_swapChain)));
		core::release(swapChain);

		m_currentBBIndex = m_swapChain->GetCurrentBackBufferIndex();

		for (u32 i{ 0 }; i < frame_buffer_count; i++)
		{
			m_renderTargetData[i].rtv = core::rtv_heap().allocate();
		}

		Finalize();
	}

	void d3d12_surface::present() const
	{
		assert(m_swapChain);
		DXCall(m_swapChain->Present(0, m_presentFlags));
		m_currentBBIndex = m_swapChain->GetCurrentBackBufferIndex();
	}

	void d3d12_surface::resize()
	{
		assert(m_swapChain);
		for (u32 i{ 0 }; i < bufferCount; i++)
		{
			core::release(m_renderTargetData[i].resource);
		}

		const u32 flags{ m_allowTearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0ul };
		DXCall(m_swapChain->ResizeBuffers(bufferCount, 0, 0, DXGI_FORMAT_UNKNOWN, flags));
		m_currentBBIndex = m_swapChain->GetCurrentBackBufferIndex();

		Finalize();

		DEBUG_OP(OutputDebugString(L"::D3D12 surface Resized.\n"));
	}

	// PRIVATE
	void d3d12_surface::Finalize()
	{
		// Create RTVs for back-buffers
		for (u32 i{ 0 }; i < bufferCount; i++)
		{
			RenderTargetData& data{ m_renderTargetData[i] };
			assert(!data.resource);
			DXCall(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&data.resource)));
			D3D12_RENDER_TARGET_VIEW_DESC desc{};
			desc.Format = m_format;
			desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
			core::device()->CreateRenderTargetView(data.resource, &desc, data.rtv.cpu);
		}

		DXGI_SWAP_CHAIN_DESC desc{};
		DXCall(m_swapChain->GetDesc(&desc));
		const u32 width{ desc.BufferDesc.Width };
		const u32 height{ desc.BufferDesc.Height };
		assert(m_window.width() == width && m_window.height() == height);

		// Set viewport and scissor rectangle
		m_viewport.TopLeftX = 0.0f;
		m_viewport.TopLeftY = 0.0f;
		m_viewport.Width = (float)width;
		m_viewport.Height = (float)height;
		m_viewport.MinDepth = 0.0f;
		m_viewport.MaxDepth = 1.0f;

		m_scissorRect = { 0, 0, (s32)width, (s32)height };
	}
	
	void d3d12_surface::release()
	{
		for (u32 i{ 0 }; i < bufferCount; i++)
		{
			RenderTargetData& data{ m_renderTargetData[i] };
			core::release(data.resource);
			core::rtv_heap().free(data.rtv);
		}

		core::release(m_swapChain);
	}
}
