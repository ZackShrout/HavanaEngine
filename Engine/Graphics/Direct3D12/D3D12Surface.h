#pragma once
#include "D3D12CommonHeaders.h"
#include "D3D12Resources.h"

namespace Havana::Graphics::D3D12
{
	class D3D12Surface
	{
	public:
		explicit D3D12Surface(Platform::Window window) : m_window{ window }
		{
			assert(m_window.Handle());
		}
		~D3D12Surface() { Release(); }

		void CreateSwapChain(IDXGIFactory7* factory, ID3D12CommandQueue* cmdQueue, DXGI_FORMAT format);
		void Present() const;
		void Resize();
		constexpr u32 Width() const { return (u32)m_viewport.Width; }
		constexpr u32 Height() const { return (u32)m_viewport.Height; }
		constexpr ID3D12Resource* const BackBuffer() const { return m_renderTargetData[m_currentBBIndex].resource; }
		constexpr D3D12_CPU_DESCRIPTOR_HANDLE RTV() const { return m_renderTargetData[m_currentBBIndex].rtv.cpu; }
		constexpr const D3D12_VIEWPORT& Viewport() const { return m_viewport; }
		constexpr const D3D12_RECT& ScissorRect() const { return m_scissorRect; }

	private:
		struct RenderTargetData
		{
			ID3D12Resource* resource{ nullptr };
			DescriptorHandle rtv{};
		};

		Platform::Window	m_window{};
		IDXGISwapChain4*	m_swapChain{ nullptr };
		RenderTargetData	m_renderTargetData[frameBufferCount]{};
		mutable u32			m_currentBBIndex{ 0 };
		D3D12_VIEWPORT		m_viewport{ 0 };
		D3D12_RECT			m_scissorRect{ 0 };

		void Finalize();
		void Release();
	};
}