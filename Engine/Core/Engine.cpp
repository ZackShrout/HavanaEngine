#ifndef SHIPPING

#include<thread>
#include "..\Content\ContentLoader.h"
#include "..\Components\Script.h"

bool EngineInitialize()
{
	bool result{ Havana::Content::LoadGame() };
	return result;
}

void EngineUpdate()
{
	Havana::Script::Update(10.0f);
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

void EngineShutdown()
{
	Havana::Content::UnloadGame();
}

#endif // !SHIPPING