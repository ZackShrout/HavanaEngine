#pragma once
#include "..\Components\ComponentsCommon.h"

namespace Havana::Script
{
	DEFINE_TYPED_ID(script_id);

	class Component final
	{
	public:
		constexpr Component() : id{ Id::INVALID_ID } {}
		constexpr explicit Component(script_id id) : id{ id } {}
		constexpr script_id GetID() const { return id; }
		constexpr bool IsValid() const { return Id::IsValid(id); }

	private:
		script_id id;
	};
}