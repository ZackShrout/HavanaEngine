#pragma once
#include "ComponentsCommon.h"

namespace Havana::Script
{
	struct InitInfo
	{
		Detail::script_creator script_creator;
	};

	Component Create(InitInfo info, Entity::Entity entity);
	void Remove(Component component);
}