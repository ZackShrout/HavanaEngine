#pragma once
#include <stdint.h>

// UNSIGNED INTEGERS
using u64 = uint64_t;
using u32 = uint32_t;
using u16 = uint16_t;
using u8 = uint8_t;

// SIGNED INTEGERS
using s64 = int64_t;
using s32 = int32_t;
using s16 = int16_t;
using s8 = int8_t;

// FLOATS
using f32 = float;

#ifdef _WIN64
// CONSTANTS
constexpr u64 U64_INVALID_ID{ 0xffff'ffff'ffff'ffffui64 };
constexpr u32 U32_INVALID_ID{ 0xffff'ffffui32 };
constexpr u16 U16_INVALID_ID{ 0xffffui16 };
constexpr u8 U8_INVALID_ID{ 0xffui8 };
#else
constexpr u64 operator"" _ui64(unsigned long long X) noexcept { return static_cast<u64>(X); }
constexpr u32 operator"" _ui32(unsigned long long X) noexcept { return static_cast<u32>(X); }
constexpr u16 operator"" _ui16(unsigned long long X) noexcept { return static_cast<u16>(X); }
constexpr u8  operator"" _ui8(unsigned long long X)  noexcept { return static_cast<u8>(X); }

// CONSTANTS
constexpr u64 U64_INVALID_ID{ 0xffff'ffff'ffff'ffff_ui64 };
constexpr u32 U32_INVALID_ID{ 0xffff'ffff_ui32 };
constexpr u16 U16_INVALID_ID{ 0xffff_ui16 };
constexpr u8 U8_INVALID_ID{ 0xff_ui8 };
#endif // _WIN64