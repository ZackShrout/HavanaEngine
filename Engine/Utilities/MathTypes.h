#pragma once
#include "CommonHeaders.h"
namespace havana::math
{
	constexpr float pi = 3.1415926535897932384626433832795f;
	constexpr float two_pi = 2.0f * pi;
	constexpr float epsilon = 1e-5f;

#if defined (_WIN64)
	using v2 = DirectX::XMFLOAT2;
	using v2a = DirectX::XMFLOAT2A;
	using v3 = DirectX::XMFLOAT3;
	using v3a = DirectX::XMFLOAT3A;
	using v4 = DirectX::XMFLOAT4;
	using v4a = DirectX::XMFLOAT4A;
	using u32v2 = DirectX::XMUINT2;
	using u32v3 = DirectX::XMUINT3;
	using u32v4 = DirectX::XMUINT4;
	using s32v2 = DirectX::XMINT2;
	using s32v3 = DirectX::XMINT3;
	using s32v4 = DirectX::XMINT4;
	using m3x3 = DirectX::XMFLOAT3X3;
	using m4x4 = DirectX::XMFLOAT4X4;
	using m4x4a = DirectX::XMFLOAT4X4A;
#endif

#if defined (__linux__)
	using v2 = glm::vec2;
	using v2a = glm::vec2;
	using v3 = glm::vec3;
	using v3a = glm::vec3;
	using v4 = glm::vec4;
	using v4a = glm::vec4;
	using u32v2 = glm::u32vec2;
	using u32v3 = glm::u32vec3;
	using u32v4 = glm::uvec4;
	using s32v2 = glm::ivec2;
	using s32v3 = glm::ivec3;
	using s32v4 = glm::ivec4;
	using m3x3 = glm::mat3;
	using m4x4 = glm::mat4;
	using m4x4a = glm::mat4;
#endif

}