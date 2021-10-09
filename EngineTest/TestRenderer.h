#pragma once
#include "Test.h"

class EngineTest : public Test
{
public:
#ifdef _WIN64
	void Run() override;
#elif __linux__
	void Run(void* disp) override;
#endif
	bool Initialize(void* disp) override;
	void Shutdown() override;
};