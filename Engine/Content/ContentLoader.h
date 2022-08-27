#pragma once
#include "CommonHeaders.h"

#ifndef SHIPPING
namespace havana::Content
{
	bool LoadGame();
	void UnloadGame();

	bool LoadEngineShaders(std::unique_ptr<u8[]>& shaders, u64& size);
}
#endif // !SHIPPING