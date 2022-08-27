#pragma once
#include "ComponentsCommon.h"

namespace havana
{

#define INIT_INFO(component) namespace component { struct InitInfo; }
	INIT_INFO(Transform);
	INIT_INFO(Script);
#undef INIT_INFO

	namespace Entity
	{
		struct EntityInfo
		{
			Transform::InitInfo* transform{ nullptr };
			Script::InitInfo* script{ nullptr };
		};

		Entity CreateEntity(EntityInfo info);
		void RemoveEntity(entity_id id);
		bool IsAlive(entity_id id);
	}
}