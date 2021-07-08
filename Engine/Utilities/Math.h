#pragma once

#include "..\Common\CommonHeaders.h"
#include "MathTypes.h"

namespace Havana::Math
{
	template<typename T>
	constexpr T clamp(T value, T min, T max)
	{
		return (value < min) ? min : (value > max) ? max : value;
	}

	template<u32 bits>
	constexpr u32 PackFloat(f32 f)
	{
		static_assert(bits <= sizeof(u32) * 8);
		assert(f >= 0.0f && f <= 1.0f);

		constexpr u32 intervals{ (u32)((1ui32 << bits) - 1) };

		return (u32)(intervals * f + 0.5f);
	}

	template<u32 bits>
	constexpr f32 UnpackToUnitFloat(u32 i)
	{
		static_assert(bits <= sizeof(u32) * 8);
		assert(i < (1ui32 << bits));

		constexpr u32 intervals{ (u32)((1ui32 << bits) - 1) };

		return (f32)i / intervals;
	}

	template<u32 bits>
	constexpr u32 PackFloat(f32 f, f32 min, f32 max)
	{
		assert(min < max);
		assert(f <= max && f >= min);

		// Scale f to a value between 0 and 1
		const f32 distance{ (f - min) / (max - min) };

		return PackFloat<bits>(distance);
	}

	template<u32 bits>
	constexpr f32 UnpackToUnitFloat(u32 i, f32 min, f32 max)
	{
		assert(max < min);

		return UnpackToUnitFloat<bits>(i) * (max - min) + min;
	}
}





