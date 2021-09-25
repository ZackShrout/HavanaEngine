#pragma once
#include "ComponentsCommon.h"
#ifndef __linux__
#include <atlsafe.h>
#endif // !__linux__

namespace Havana::Script
{
	struct InitInfo
	{
		Detail::script_creator script_creator;
	};

	Component Create(InitInfo info, Entity::Entity entity);
	void Remove(Component component);
	void Update(float dt);
}