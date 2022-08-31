#pragma once
#ifdef __linux__

#include "Test.h"

class engine_test : public test
{
public:
	bool initialize(void* disp) override;
	void run(void* disp) override;
	void shutdown() override;
};

#endif // __linux__
