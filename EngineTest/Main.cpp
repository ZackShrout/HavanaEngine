#include "Test.h"
#pragma comment(lib, "engine.lib")

// What test are we performing?
#define TEST_ENTITY_COMPONENTS 1

#if TEST_ENTITY_COMPONENTS
#include "TestEntityComponents.h"
#else
#error One of the tests must be enabled
#endif

int main()
{

#if _DEBUG
	// Debug flags that help check for memory leaks
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	EngineTest test{};

	if (test.Initialize())
	{
		test.Run();
	}

	test.Shutdown();

	return 0;
}