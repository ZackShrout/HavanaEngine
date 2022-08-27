#pragma once
#include "D3D12CommonHeaders.h"

namespace havana::Graphics::D3D12::FX
{
	bool Initialize();
	void Shutdown();

	void PostProcess(ID3D12GraphicsCommandList* cmdList, D3D12_CPU_DESCRIPTOR_HANDLE targetRTV);
}