#pragma once
#include "CommonHeaders.h"

#if !defined(SHIPPING) && defined(_WIN64)
namespace havana::content
{
	bool load_game();
	void unload_game();

	bool load_engine_shaders(std::unique_ptr<u8[]>& shaders, u64& size);
}
#endif // !defined(SHIPPING)