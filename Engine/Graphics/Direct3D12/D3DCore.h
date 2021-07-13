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
}