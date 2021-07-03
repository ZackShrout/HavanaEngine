#pragma once
#include "..\Common\CommonHeaders.h"

#ifndef SHIPPING
namespace Havana::Content
{
	bool LoadGame();
	void UnloadGame();
}
#endif // !SHIPPING