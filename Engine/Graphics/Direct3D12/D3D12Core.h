#pragma once
#include "D3D12CommonHeaders.h"

namespace Havana::Graphics::D3D12
{
	class DescriptorHeap;
}

namespace Havana::Graphics::D3D12::Core
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

	namespace Detail
	{
		void DeferredRelease(IUnknown* resource);
	}

	template<typename T>
	constexpr void DeferredRelease(T*& resource)
	{
		if (resource)
		{
			Detail::DeferredRelease(resource);
			resource = nullptr;
		}
	}

	ID3D12Device* const Device();
	DescriptorHeap& RTVHeap();
	DescriptorHeap& DSVHeap();
	DescriptorHeap& SRVHeap();
	DescriptorHeap& UAVHeap();
	DXGI_FORMAT DefaultRenderTargetFormat();
	u32 CurrentFrameIndex();
	void SetDeferredReleasesFlag();

	Surface CreateSurface(Platform::Window window);
	void RemoveSurface(surface_id id);
	void ResizeSurface(surface_id id, u32, u32);
	u32 SurfaceWidth(surface_id id);
	u32 SurfaceHeight(surface_id id);
	void RenderSurface(surface_id id);
}