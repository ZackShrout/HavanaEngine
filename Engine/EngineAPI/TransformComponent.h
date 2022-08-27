#pragma once
#include "../Components/ComponentsCommon.h"

namespace havana::Transform
{
	DEFINE_TYPED_ID(transform_id);
	
	class Component final
	{
	public:
		constexpr Component() : m_id{ id::invalid_id } {}
		constexpr explicit Component(transform_id id) : m_id{ id } {}		
		constexpr transform_id GetID() const { return m_id; }
		constexpr bool is_valid() const { return id::is_valid(m_id); }
		math::v4 Rotation() const;
		math::v3 Orientation() const;
		math::v3 Position() const;
		math::v3 Scale() const;
	private:
		transform_id m_id;
	};
}