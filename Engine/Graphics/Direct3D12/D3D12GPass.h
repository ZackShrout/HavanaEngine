#pragma once
#include "D3D12CommonHeaders.h"

namespace havana::graphics::d3d12
{
	struct D3D12FrameInfo;	// forward declaration
}

namespace havana::graphics::d3d12::GPass
{
	bool initialize();
	void shutdown();

	[[nodiscard]] const D3D12RenderTexture& MainBuffer();
	[[nodiscard]] const D3D12DepthBuffer& DepthBuffer();

	// NOTE: call this every frame before rendering anything in gpass
	void SetSize(math::u32v2 size);
	void DepthPrepass(ID3D12GraphicsCommandList* cmdList, const D3D12FrameInfo& info);
	void render(ID3D12GraphicsCommandList* cmdList, const D3D12FrameInfo& info);

	void AddTransitionsForDepthPrepass(D3DX::ResourceBarrier& barriers);
	void AddTransitionsForGPass(D3DX::ResourceBarrier& barriers);
	void AddTransitionsForPostProcess(D3DX::ResourceBarrier& barriers);

	void SetRenderTargetsForDepthPrepass(id3d12GraphicsCommandList* cmdList);
	void SetRenderTargetsForGPass(id3d12GraphicsCommandList* cmdList);
}