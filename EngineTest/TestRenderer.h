#pragma once
#include "Test.h"

class EngineTest : public Test
{
public:
	bool Initialize() override;
	void Run() override;
	void Shutdown() override;
};