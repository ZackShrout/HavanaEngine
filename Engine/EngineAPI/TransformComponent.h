#pragma once
#include "../Components/ComponentsCommon.h"

namespace Havana::Transform
{
	DEFINE_TYPED_ID(transform_id);
	
	class Component final
	{
	public:
		constexpr Component() : m_id{ Id::INVALID_ID } {}
		constexpr explicit Component(transform_id id) : m_id{ id } {}		
		constexpr transform_id GetID() const { return m_id; }
		constexpr bool IsValid() const { return Id::IsValid(m_id); }
		Math::Vec4 Rotation() const;
		Math::Vec3 Orientation() const;
		Math::Vec3 Position() const;
		Math::Vec3 Scale() const;
	private:
		transform_id m_id;
	};
}