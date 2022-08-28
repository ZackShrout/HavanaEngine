#pragma once
#include "../Components/ComponentsCommon.h"

namespace havana::transform
{
	DEFINE_TYPED_ID(transform_id);
	
	class component final
	{
	public:
		constexpr component() : m_id{ id::invalid_id } {}
		constexpr explicit component(transform_id id) : m_id{ id } {}		
		constexpr transform_id get_id() const { return m_id; }
		constexpr bool is_valid() const { return id::is_valid(m_id); }
		math::v4 rotation() const;
		math::v3 orientation() const;
		math::v3 position() const;
		math::v3 scale() const;
	private:
		transform_id m_id;
	};
}