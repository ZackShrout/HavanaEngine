#pragma once
#include "EngineAPI/Input.h"
#include "CommonHeaders.h"

namespace havana::input
{
	void set(input_source::type type, input_code::code code, math::v3 value);
}