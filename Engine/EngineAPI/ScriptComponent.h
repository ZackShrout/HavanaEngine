#pragma once
#include "../Components/ComponentsCommon.h"

namespace Havana::Script
{
	DEFINE_TYPED_ID(script_id);

	class Component final
	{
	public:
		constexpr Component() : m_id{ Id::INVALID_ID } {}
		constexpr explicit Component(script_id id) : m_id{ id } {}
		constexpr script_id GetID() const { return m_id; }
		constexpr bool IsValid() const { return Id::IsValid(m_id); }

	private:
		script_id m_id;
	};
}