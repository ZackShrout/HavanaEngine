#pragma once
#include "ComponentsCommon.h"

namespace Havana::Transform
{
	DEFINE_TYPED_ID(transform_id);

	struct InitInfo
	{
		f32 position[3]{};
		f32 rotation[4]{};
		f32 scale[3]{ 1.f, 1.f, 1.f };
	};

	transform_id CreateTransform(const InitInfo& info, Entity::entity_id entityID);
	void RemoveTransform(transform_id id);
}