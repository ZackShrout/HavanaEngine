#pragma once
#include "../Components/ComponentsCommon.h"

namespace havana::script
{
	DEFINE_TYPED_ID(script_id);

	class component final
	{
	public:
		constexpr component() : m_id{ id::invalid_id } {}
		constexpr explicit component(script_id id) : m_id{ id } {}
		constexpr script_id get_id() const { return m_id; }
		constexpr bool is_valid() const { return id::is_valid(m_id); }

	private:
		script_id m_id;
	};
}