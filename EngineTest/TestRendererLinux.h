#pragma once
#ifdef __linux__

#include "Test.h"

class EngineTest : public Test
{
public:
	bool initialize(void* disp) override;
	void Run(void* disp) override;
	void shutdown() override;
};

#endif // __linux__
