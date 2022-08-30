#pragma once
#include "D3D12CommonHeaders.h"

namespace havana::graphics::d3d12
{
	class d3d12_surface
	{
	public:
		constexpr static u32 bufferCount{ 3 };
		constexpr static DXGI_FORMAT defaultBackBufferFormat{ DXGI_FORMAT_R8G8B8A8_UNORM_SRGB };

		explicit d3d12_surface(platform::window window) : m_window{ window }
		{
			assert(m_window.handle());
		}
#if USE_STL_VECTOR
		DISABLE_COPY(d3d12_surface);
		constexpr d3d12_surface(d3d12_surface&& o)
			: m_swapChain{ o.m_swapChain }, m_window{ o.m_window }, m_currentBBIndex{ o.m_currentBBIndex },
			m_viewport{ o.m_viewport }, m_scissorRect{ o.m_scissorRect }, m_allowTearing{ o.m_allowTearing },
			m_presentFlags{ o.m_presentFlags }
		{
			for (u32 i{ 0 }; i < frame_buffer_count; i++)
			{
				m_renderTargetData[i].resource = o.m_renderTargetData[i].resource;
				m_renderTargetData[i].rtv = o.m_renderTargetData[i].rtv;
			}

			o.reset();
		}

		constexpr d3d12_surface& operator=(d3d12_surface&& o)
		{
			assert(this != &o);
			if (this != &o)
			{
				release();
				move(o);
			}

			return *this;
		}
#else
		DISABLE_COPY_AND_MOVE(d3d12_surface);
#endif // USE_STL_VECTOR
		~d3d12_surface() { release(); }

		void CreateSwapChain(IDXGIFactory7* factory, ID3D12CommandQueue* cmdQueue, DXGI_FORMAT format = defaultBackBufferFormat);
		void present() const;
		void resize();
		constexpr u32 width() const { return (u32)m_viewport.Width; }
		constexpr u32 height() const { return (u32)m_viewport.Height; }
		constexpr ID3D12Resource* const BackBuffer() const { return m_renderTargetData[m_currentBBIndex].resource; }
		constexpr D3D12_CPU_DESCRIPTOR_HANDLE rtv() const { return m_renderTargetData[m_currentBBIndex].rtv.cpu; }
		constexpr const D3D12_VIEWPORT& viewport() const { return m_viewport; }
		constexpr const D3D12_RECT& scissor_rect() const { return m_scissorRect; }

	private:
		struct RenderTargetData
		{
			ID3D12Resource* resource{ nullptr };
			descriptor_handle rtv{};
		};

		platform::window	m_window{};
		DXGI_FORMAT			m_format{ defaultBackBufferFormat };
		IDXGISwapChain4*	m_swapChain{ nullptr };
		RenderTargetData	m_renderTargetData[bufferCount]{};
		mutable u32			m_currentBBIndex{ 0 };
		u32					m_allowTearing{ 0 };
		u32					m_presentFlags{ 0 };
		D3D12_VIEWPORT		m_viewport{ 0 };
		D3D12_RECT			m_scissorRect{ 0 };

		void Finalize();
		void release();

#if USE_STL_VECTOR
		constexpr void reset()
		{
			m_window = {};
			m_swapChain = nullptr;
			for (u32 i{ 0 }; i < bufferCount; i++)
			{
				m_renderTargetData[i] = {};
			}
			m_currentBBIndex = 0;
			m_allowTearing = 0;
			m_presentFlags = 0;
			m_viewport = {};
			m_scissorRect = {};
		}

		constexpr void move(d3d12_surface& o)
		{
			m_window = o.m_window;
			m_swapChain = o.m_swapChain;
			for (u32 i{ 0 }; i < frame_buffer_count; i++)
			{
				m_renderTargetData[i] = o.m_renderTargetData[i];
			}
			m_currentBBIndex = o.m_currentBBIndex;
			m_allowTearing = o.m_allowTearing;
			m_presentFlags = o.m_presentFlags;
			m_viewport = o.m_viewport;
			m_scissorRect = o.m_scissorRect;

			o.reset();
		}
#endif // USE_STL_VECTOR
	};
}