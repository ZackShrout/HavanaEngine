#pragma once
#include "D3D12CommonHeaders.h"

namespace havana::graphics::d3d12::fx
{
	bool initialize();
	void shutdown();

	void post_process(ID3D12GraphicsCommandList* cmdList, D3D12_CPU_DESCRIPTOR_HANDLE targetRTV);
}