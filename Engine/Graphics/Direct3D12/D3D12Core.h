#pragma once
#include "D3D12CommonHeaders.h"

namespace havana::graphics::d3d12
{
	struct D3D12FrameInfo
	{
		u32 surfaceWidth{};
		u32 surfaceHeight{};
	};
}

namespace havana::graphics::d3d12::Core
{
	bool Initialize();
	void Shutdown();

	template<typename T>
	constexpr void Release(T*& resource)
	{
		if (resource)
		{
			resource->Release();
			resource = nullptr;
		}
	}

	namespace detail
	{
		void DeferredRelease(IUnknown* resource);
	}

	template<typename T>
	constexpr void DeferredRelease(T*& resource)
	{
		if (resource)
		{
			detail::DeferredRelease(resource);
			resource = nullptr;
		}
	}

	id3d12Device* const Device();
	DescriptorHeap& RTVHeap();
	DescriptorHeap& DSVHeap();
	DescriptorHeap& SRVHeap();
	DescriptorHeap& UAVHeap();
	u32 CurrentFrameIndex();
	void SetDeferredReleasesFlag();

	surface CreateSurface(platform::window window);
	void RemoveSurface(surface_id id);
	void ResizeSurface(surface_id id, u32, u32);
	u32 SurfaceWidth(surface_id id);
	u32 SurfaceHeight(surface_id id);
	void RenderSurface(surface_id id);
}