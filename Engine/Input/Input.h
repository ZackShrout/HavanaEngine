#pragma once
#include "EngineAPI/Input.h"
#include "CommonHeaders.h"

namespace havana::input
{
	void bind(input_source source);
	void unbind(input_source::type type, input_code::code code);
	void unbind(u64 binding);
	void set(input_source::type type, input_code::code code, math::v3 value);
}