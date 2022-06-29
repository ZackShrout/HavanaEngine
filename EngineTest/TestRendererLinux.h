#pragma once
#ifdef __linux__

#include "Test.h"

class EngineTest : public Test
{
public:
	bool Initialize(void* disp) override;
	void Run(void* disp) override;
	void Shutdown() override;
};

#endif // __linux__
