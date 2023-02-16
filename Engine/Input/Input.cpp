#include "Input.h"

namespace havana::input
{
	namespace
	{
		std::unordered_map<u64, input_value>	input_values;
		utl::vector<detail::input_system_base*>	input_callbacks;

		constexpr u64
		get_key(input_source::type type, u32 code)
		{
			return ((u64)type << 32) | (u64)code;
		}
	} // anonymous namespace

	void
	set(input_source::type type, input_code::code code, math::v3 value)
	{
		assert(type < input_source::count);
		const u64 key{ get_key(type, code) };
		input_value& input{ input_values[key] };
		input.previous = input.current;
		input.current = value;

		// TODO: these callbacks could cause data-races in scripts when not run on the same thread as game scripts
		for (const auto& c : input_callbacks)
		{
			c->on_event(type, code, input);
		}
	}

	void
	get(input_source::type type, input_code::code code, input_value& value)
	{
		assert(type < input_source::count);
		const u64 key{ get_key(type, code) };
		value = input_values[key];
	}

	detail::input_system_base::input_system_base()
	{
		input_callbacks.emplace_back(this);
	}

	detail::input_system_base::~input_system_base()
	{
		for (u32 i{ 0 }; i < input_callbacks.size(); ++i)
		{
			if (input_callbacks[i] == this)
			{
				utl::erase_unordered(input_callbacks, i);
				break;
			}
		}
	}
}