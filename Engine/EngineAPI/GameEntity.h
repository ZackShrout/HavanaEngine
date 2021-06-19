#pragma once
#include "..\Components\ComponentsCommon.h"
#include "TransformComponent.h"

namespace Havana::Entity
{
	DEFINE_TYPED_ID(entity_id);

	class Entity
	{
	public:
		constexpr Entity() : id{ Id::INVALID_ID } {}
		constexpr explicit Entity(entity_id id) : id{ id } {}				
		constexpr entity_id GetID() const { return id; }
		constexpr bool IsValid() const { return Id::IsValid(id); }
		Transform::Component Transform() const;
	private:
		entity_id id;
	};
}