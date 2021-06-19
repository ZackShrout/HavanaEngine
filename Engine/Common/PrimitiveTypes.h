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

// CONSTANTS
constexpr u64 U64_INVALID_ID{0xffff'ffff'ffff'ffffui64};
constexpr u32 U32_INVALID_ID{ 0xffff'ffffui32 };
constexpr u16 U16_INVALID_ID{ 0xffffui16 };
constexpr u8 U8_INVALID_ID{ 0xffui8 };