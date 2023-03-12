#pragma once
//#include "CommonHeaders.h"
#include "EngineAPI/Input.h"

namespace havana::input
{
	void set(input_source::type type, input_code::code code, math::v3 value);
}