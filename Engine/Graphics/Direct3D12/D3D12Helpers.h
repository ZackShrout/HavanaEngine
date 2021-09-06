#pragma once
#include "D3D12CommonHeaders.h"

namespace Havana::Graphics::D3D12::D3DX
{
	
	constexpr struct
	{
		D3D12_HEAP_PROPERTIES defaultHeap
		{
			D3D12_HEAP_TYPE_DEFAULT,			// Type;	
			D3D12_CPU_PAGE_PROPERTY_UNKNOWN,	// CPUPageProperty;
			D3D12_MEMORY_POOL_UNKNOWN,			// MemoryPoolPreference;
			0,									// CreationNodeMask;
			0									// VisibleNodeMask;
		};

	} heapProperties;
}