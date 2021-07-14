#pragma once
#include "D3D12CommonHeaders.h"

namespace Havana::Graphics::D3D12::Core
{
	bool Initialize();
	void Shutdown();
	void Render();

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
	u32 CurrentFrameIndex();
	void SetDeferredReleasesFlag();
}