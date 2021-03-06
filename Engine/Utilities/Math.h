#pragma once

#include "CommonHeaders.h"
#include "MathTypes.h"

namespace Havana::Math
{
	template<typename T>
	constexpr T clamp(T value, T min, T max)
	{
		return (value < min) ? min : (value > max) ? max : value;
	}

	template<u32 bits>
	constexpr u32 PackUnitFloat(f32 f)
	{
		static_assert(bits <= sizeof(u32) * 8);
		assert(f >= 0.0f && f <= 1.0f);
	#ifdef _WIN64
		constexpr u32 intervals{ (u32)((1ui32 << bits) - 1) };
	#else
		constexpr u32 intervals{ (u32)((1_ui32 << bits) - 1) };
	#endif

		return (u32)(intervals * f + 0.5f);
	}

	template<u32 bits>
	constexpr f32 UnpackToUnitFloat(u32 i)
	{
		static_assert(bits <= sizeof(u32) * 8);
	#ifdef _WIN64
		assert(i < (1ui32 << bits));
		constexpr u32 intervals{ (u32)((1ui32 << bits) - 1) };
	#else
		assert(i < (1_ui32 << bits));
		constexpr u32 intervals{ (u32)((1_ui32 << bits) - 1) };
	#endif

		return (f32)i / intervals;
	}

	template<u32 bits>
	constexpr u32 PackFloat(f32 f, f32 min, f32 max)
	{
		assert(min < max);
		assert(f <= max && f >= min);

		// Scale f to a value between 0 and 1
		const f32 distance{ (f - min) / (max - min) };

		return PackUnitFloat<bits>(distance);
	}

	template<u32 bits>
	constexpr f32 UnpackToFloat(u32 i, f32 min, f32 max)
	{
		assert(max < min);

		return UnpackToUnitFloat<bits>(i) * (max - min) + min;
	}
	
	/// <summary>
	/// Align by rounding up. Will result in a multiple of 'alignment'
	/// that is greater than or equal to 'size'
	/// </summary>
	template<u64 alignment>
	constexpr u64 AlignSizeUp(u64 size)
	{
		static_assert(alignment, "Alignment must be non-zero");
		constexpr u64 mask{ alignment - 1 };
		static_assert(!(alignment & mask), "Alignment must be a power of 2.");
		return ((size + mask) & -mask);
	}

	/// <summary>
	/// Align by rounding down. Will result in a multiple of 'alignment'
	/// that is less than or equal to 'size'
	/// </summary>
	/// <param name="size"></param>
	/// <returns></returns>
	template<u64 alignment>
	constexpr u64 AlignSizeDown(u64 size)
	{
		static_assert(alignment, "Alignment must be non-zero");
		constexpr u64 mask{ alignment - 1 };
		static_assert(!(alignment & mask), "Alignment must be a power of 2.");
		return (size & -mask);
	}
}