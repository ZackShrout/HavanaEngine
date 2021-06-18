#pragma once
#include "ComponentsCommon.h"

namespace Havana
{

#define INIT_INFO(component) namespace component { struct InitInfo; }
	INIT_INFO(Transform);
#undef INIT_INFO

	namespace Entity
	{
		struct EntityInfo
		{
			Transform::InitInfo* transform{ nullptr };
		};

		entity_id CreateEntity(const EntityInfo& info);
		void RemoveEntity(entity_id id);
		bool IsAlive(entity_id id);
	}
}