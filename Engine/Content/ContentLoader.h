#pragma once
#include "CommonHeaders.h"

#ifndef SHIPPING
namespace Havana::Content
{
	bool LoadGame();
	void UnloadGame();
}
#endif // !SHIPPING