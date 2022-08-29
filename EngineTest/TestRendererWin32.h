#pragma once

#ifdef _WIN64

#include "Test.h"

class EngineTest : public Test
{
public:
	bool initialize() override;
	void Run() override;
	void shutdown() override;
};

#endif // _WIN64
