#pragma once
#include "../Components/ComponentsCommon.h"

namespace havana::Script
{
	DEFINE_TYPED_ID(script_id);

	class Component final
	{
	public:
		constexpr Component() : m_id{ id::invalid_id } {}
		constexpr explicit Component(script_id id) : m_id{ id } {}
		constexpr script_id GetID() const { return m_id; }
		constexpr bool is_valid() const { return id::is_valid(m_id); }

	private:
		script_id m_id;
	};
}