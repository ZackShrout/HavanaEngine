#pragma once
#include "..\Components\ComponentsCommon.h"

namespace Havana::Transform
{
	DEFINE_TYPED_ID(transform_id);
	
	class Component final
	{
	public:
		constexpr Component() : id{ Id::INVALID_ID } {}
		constexpr explicit Component(transform_id id) : id{ id } {}		
		constexpr transform_id GetID() const { return id; }
		constexpr bool IsValid() const { return Id::IsValid(id); }
		Math::Vec3 Position() const;
		Math::Vec4 Rotation() const;
		Math::Vec3 Scale() const;
	private:
		transform_id id;
	};
}