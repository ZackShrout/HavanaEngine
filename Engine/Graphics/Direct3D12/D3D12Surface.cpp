#include "D3D12Surface.h"
#include "D3D12Core.h"

namespace Havana::Graphics::D3D12
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
	void D3D12Surface::CreateSwapChain(IDXGIFactory7* factory, ID3D12CommandQueue* cmdQueue, DXGI_FORMAT format)
	{
		assert(factory && cmdQueue);
		Release();

		if (SUCCEEDED(factory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &m_allowTearing, sizeof(u32))) && m_allowTearing)
		{
			m_presentFlags = DXGI_PRESENT_ALLOW_TEARING;
		}

		DXGI_SWAP_CHAIN_DESC1 desc{};
		desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		desc.BufferCount = frameBufferCount;
		desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		desc.Flags = m_allowTearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
		desc.Format = ToNonSRGB(format);
		desc.Height = m_window.Height();
		desc.Width = m_window.Width();
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Scaling = DXGI_SCALING_STRETCH;
		desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		desc.Stereo = false;

		IDXGISwapChain1* swapChain;
		HWND hwnd{ (HWND)m_window.Handle() };
		DXCall(factory->CreateSwapChainForHwnd(cmdQueue, hwnd, &desc, nullptr, nullptr, &swapChain));
		DXCall(factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER));
		DXCall(swapChain->QueryInterface(IID_PPV_ARGS(&m_swapChain)));
		Core::Release(swapChain);

		m_currentBBIndex = m_swapChain->GetCurrentBackBufferIndex();

		for (u32 i{ 0 }; i < frameBufferCount; i++)
		{
			m_renderTargetData[i].rtv = Core::RTVHeap().Allocate();
		}

		Finalize();
	}

	void D3D12Surface::Present() const
	{
		assert(m_swapChain);
		DXCall(m_swapChain->Present(0, m_presentFlags));
		m_currentBBIndex = m_swapChain->GetCurrentBackBufferIndex();
	}

	void D3D12Surface::Resize()
	{
		// TODO: implement
	}

	// PRIVATE
	void D3D12Surface::Finalize()
	{
		// Create RTVs for back-buffers
		for (u32 i{ 0 }; i < frameBufferCount; i++)
		{
			RenderTargetData& data{ m_renderTargetData[i] };
			assert(!data.resource);
			DXCall(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&data.resource)));
			D3D12_RENDER_TARGET_VIEW_DESC desc{};
			desc.Format = Core::DefaultRenderTargetFormat();
			desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
			Core::Device()->CreateRenderTargetView(data.resource, &desc, data.rtv.cpu);
		}

		DXGI_SWAP_CHAIN_DESC desc{};
		DXCall(m_swapChain->GetDesc(&desc));
		const u32 width{ desc.BufferDesc.Width };
		const u32 height{ desc.BufferDesc.Height };
		assert(m_window.Width() == width && m_window.Height() == height);

		// Set viewport and scissor rectangle
		m_viewport.TopLeftX = 0.0f;
		m_viewport.TopLeftY = 0.0f;
		m_viewport.Width = (float)width;
		m_viewport.Height = (float)height;
		m_viewport.MinDepth = 0.0f;
		m_viewport.MaxDepth = 1.0f;

		m_scissorRect = { 0, 0, (s32)width, (s32)height };
	}
	
	void D3D12Surface::Release()
	{
		for (u32 i{ 0 }; i < frameBufferCount; i++)
		{
			RenderTargetData& data{ m_renderTargetData[i] };
			Core::Release(data.resource);
			Core::RTVHeap().Free(data.rtv);
		}

		Core::Release(m_swapChain);
	}
}
