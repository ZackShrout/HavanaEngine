#pragma once

#ifdef _WIN64

#include "Test.h"

class EngineTest : public Test
{
public:
	bool Initialize() override;
	void Run() override;
	void Shutdown() override;
};

#endif // _WIN64
