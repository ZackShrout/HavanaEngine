#pragma once
#include "ComponentsCommon.h"

namespace Havana::Transform
{
	

	struct InitInfo
	{
		f32 position[3]{};
		f32 rotation[4]{};
		f32 scale[3]{ 1.f, 1.f, 1.f };
	};

	Component CreateTransform(const InitInfo& info, Entity::Entity entity);
	void RemoveTransform(Component component);
}