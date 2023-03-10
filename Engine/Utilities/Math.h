#pragma once

#ifdef __linux__
#include <smmintrin.h>
#endif // __linux__

#include "CommonHeaders.h"
#include "MathTypes.h"

namespace havana::math
{
	constexpr bool
	is_equal(f32 a, f32 b, f32 eps = epsilon)
	{
		f32 diff{ a - b };
		if (diff < 0.f) diff = -diff;
		return diff < eps;
	}
	
	template<typename T>
	[[nodiscard]] constexpr T
	clamp(T value, T min, T max)
	{
		return (value < min) ? min : (value > max) ? max : value;
	}

	template<u32 bits>
	[[nodiscard]] constexpr u32
	pack_unit_float(f32 f)
	{
		static_assert(bits <= sizeof(u32) * 8);
		assert(f >= 0.0f && f <= 1.0f);
		constexpr f32 intervals{ (f32)(((u32)1 << bits) - 1) };
		return (u32)(intervals * f + 0.5f);
	}

	template<u32 bits>
	[[nodiscard]] constexpr f32
	unpack_to_unit_float(u32 i)
	{
		static_assert(bits <= sizeof(u32) * 8);
		assert(i < ((u32)1 << bits));
		constexpr f32 intervals{ (f32)(((u32)1 << bits) - 1) };
		return (f32)i / intervals;
	}

	template<u32 bits>
	[[nodiscard]] constexpr u32
	pack_float(f32 f, f32 min, f32 max)
	{
		assert(min < max);
		assert(f <= max && f >= min);

		// Scale f to a value between 0 and 1
		const f32 distance{ (f - min) / (max - min) };

		return pack_unit_float<bits>(distance);
	}

	template<u32 bits>
	[[nodiscard]] constexpr f32
	unpack_to_float(u32 i, f32 min, f32 max)
	{
		assert(max < min);

		return unpack_to_unit_float<bits>(i) * (max - min) + min;
	}
	
	/// <summary>
	/// Align by rounding up. Will result in a multiple of 'alignment'
	/// that is greater than or equal to 'size'
	/// </summary>
	template<u64 alignment>
	[[nodiscard]] constexpr u64
	align_size_up(u64 size)
	{
		static_assert(alignment, "Alignment must be non-zero");
		constexpr u64 mask{ alignment - 1 };
		static_assert(!(alignment & mask), "Alignment must be a power of 2.");
		return ((size + mask) & ~mask);
	}

	/// <summary>
	/// Align by rounding down. Will result in a multiple of 'alignment'
	/// that is less than or equal to 'size'
	/// </summary>
	/// <param name="size"></param>
	/// <returns></returns>
	template<u64 alignment>
	[[nodiscard]] constexpr u64
	align_size_down(u64 size)
	{
		static_assert(alignment, "Alignment must be non-zero");
		constexpr u64 mask{ alignment - 1 };
		static_assert(!(alignment & mask), "Alignment must be a power of 2.");
		return (size & ~mask);
	}

	/// <summary>
	/// Align by rounding up. Will result in a multiple of 'alignment'
	/// that is greater than or equal to 'size'
	/// </summary>
	[[nodiscard]] constexpr u64
	align_size_up(u64 size, u64 alignment)
	{
		assert(alignment && "Alignment must be non-zero");
		const u64 mask{ alignment - 1 };
		assert(!(alignment & mask) && "Alignment must be a power of 2.");
		return ((size + mask) & ~mask);
	}

	/// <summary>
	/// Align by rounding down. Will result in a multiple of 'alignment'
	/// that is less than or equal to 'size'
	/// </summary>
	/// <param name="size"></param>
	/// <returns></returns>
	[[nodiscard]] constexpr u64
	align_size_down(u64 size, u64 alignment)
	{
		assert(alignment && "Alignment must be non-zero");
		const u64 mask{ alignment - 1 };
		assert(!(alignment & mask) && "Alignment must be a power of 2.");
		return (size & ~mask);
	}

	[[nodiscard]] constexpr u64
	calc_crc32_u64(const u8* const data, u64 size)
	{
		assert(size >= sizeof(u64));
		u64 crc{ 0 };
		const u8* at{ data };
		const u8* const end{ data + align_size_down<sizeof(u64)>(size) };
		while (at < end)
		{
			crc = _mm_crc32_u64(crc, *((const u64*)at));
			at += sizeof(u64);
		}

		return crc;
	}
}