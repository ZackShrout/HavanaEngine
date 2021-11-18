#pragma once
#include "D3D12CommonHeaders.h"

namespace Havana::Graphics::D3D12
{
	struct D3D12FrameInfo;	// forward declaration
}

namespace Havana::Graphics::D3D12::GPass
{
	bool Initialize();
	void Shutdown();

	// NOTE: call this every frame before rendering anything in gpass
	void SetSize(Math::Vec2u32 size);
	void DepthPrepass(ID3D12GraphicsCommandList* cmdList, const D3D12FrameInfo& info);
	void Render(ID3D12GraphicsCommandList* cmdList, const D3D12FrameInfo& info);

	void AddTransitionsForDepthPrepass(D3DX::ResourceBarrier& barriers);
	void AddTransitionsForGPass(D3DX::ResourceBarrier& barriers);
	void AddTransitionsForPostProcess(D3DX::ResourceBarrier& barriers);

	void SetRenderTargetsForDepthPrepass(id3d12GraphicsCommandList* cmdList);
	void SetRenderTargetsForGPass(id3d12GraphicsCommandList* cmdList);
}