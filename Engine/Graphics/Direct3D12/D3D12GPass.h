#pragma once
#include "D3D12CommonHeaders.h"

namespace Havana::Graphics::D3D12::GPass
{
	bool Initialize();
	void Shutdown();

	// NOTE: call this every frame before rendering anything in gpass
	void SetSize(Math::Vec2u32 size);
}