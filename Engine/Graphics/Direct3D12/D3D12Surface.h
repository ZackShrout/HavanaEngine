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
#if USE_STL_VECTOR
		DISABLE_COPY(D3D12Surface);
		constexpr D3D12Surface(D3D12Surface&& o)
			: m_swapChain{ o.m_swapChain }, m_window{ o.m_window }, m_currentBBIndex{ o.m_currentBBIndex },
			m_viewport{ o.m_viewport }, m_scissorRect{ o.m_scissorRect }, m_allowTearing{ o.m_allowTearing },
			m_presentFlags{ o.m_presentFlags }
		{
			for (u32 i{ 0 }; i < frameBufferCount; i++)
			{
				m_renderTargetData[i].resource = o.m_renderTargetData[i].resource;
				m_renderTargetData[i].rtv = o.m_renderTargetData[i].rtv;
			}

			o.Reset();
		}

		constexpr D3D12Surface& operator=(D3D12Surface&& o)
		{
			assert(this != &o);
			if (this != &o)
			{
				Release();
				Move(o);
			}

			return *this;
		}
#endif
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
		u32					m_allowTearing{ 0 };
		u32					m_presentFlags{ 0 };
		D3D12_VIEWPORT		m_viewport{ 0 };
		D3D12_RECT			m_scissorRect{ 0 };

		void Finalize();
		void Release();

#if USE_STL_VECTOR
		constexpr void Reset()
		{
			m_window = {};
			m_swapChain = nullptr;
			for (u32 i{ 0 }; i < frameBufferCount; i++)
			{
				m_renderTargetData[i] = {};
			}
			m_currentBBIndex = 0;
			m_allowTearing = 0;
			m_presentFlags = 0;
			m_viewport = {};
			m_scissorRect = {};
		}

		constexpr void Move(D3D12Surface& o)
		{
			m_window = o.m_window;
			m_swapChain = o.m_swapChain;
			for (u32 i{ 0 }; i < frameBufferCount; i++)
			{
				m_renderTargetData[i] = o.m_renderTargetData[i];
			}
			m_currentBBIndex = o.m_currentBBIndex;
			m_allowTearing = o.m_allowTearing;
			m_presentFlags = o.m_presentFlags;
			m_viewport = o.m_viewport;
			m_scissorRect = o.m_scissorRect;

			o.Reset();
		}
#endif
	};
}