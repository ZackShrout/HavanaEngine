#pragma once
#include "ComponentsCommon.h"

namespace havana::Transform
{
	

	struct InitInfo
	{
		f32 position[3]{};
		f32 rotation[4]{};
		f32 scale[3]{ 1.f, 1.f, 1.f };
	};

	Component Create(InitInfo info, Entity::Entity entity);
	void Remove(Component component);
}