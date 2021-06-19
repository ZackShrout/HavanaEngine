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

		Entity CreateEntity(const EntityInfo& info);
		void RemoveEntity(Entity id);
		bool IsAlive(Entity id);
	}
}