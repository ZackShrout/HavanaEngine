#pragma once
#include "../Common/CommonHeaders.h"
namespace Havana::Math
{
	constexpr float pi = 3.1415926535897932384626433832795f;
	constexpr float twoPi = 2.0f * pi;
	constexpr float epsilon = 1e-5f;

#if defined (_WIN64)
	using Vec2 = DirectX::XMFLOAT2;
	using Vec2A = DirectX::XMFLOAT2A;
	using Vec3 = DirectX::XMFLOAT3;
	using Vec3A = DirectX::XMFLOAT3A;
	using Vec4 = DirectX::XMFLOAT4;
	using Vec4A = DirectX::XMFLOAT4A;
	using Vec2u32 = DirectX::XMUINT2;
	using Vec3u32 = DirectX::XMUINT3;
	using Vec4u32 = DirectX::XMUINT4;
	using Vec2s32 = DirectX::XMINT2;
	using Vec3s32 = DirectX::XMINT3;
	using Vec4s32 = DirectX::XMINT4;
	using Mat3 = DirectX::XMFLOAT3X3;
	using Mat4 = DirectX::XMFLOAT4X4;
	using Mat4A = DirectX::XMFLOAT4X4A;
#endif

#if defined (__linux__)
	using Vec2 = glm::vec2;
	using Vec2A = glm::vec2;
	using Vec3 = glm::vec3;
	using Vec3A = glm::vec3;
	using Vec4 = glm::vec4;
	using Vec4A = glm::vec4;
	using Vec2u32 = glm::u32vec2;
	using Vec3u32 = glm::u32vec3;
	using Vec4u32 = glm::uvec4;
	using Vec2s32 = glm::ivec2;
	using Vec3s32 = glm::ivec3;
	using Vec4s32 = glm::ivec4;
	using Mat3 = glm::mat3;
	using Mat4 = glm::mat4;
	using Mat4A = glm::mat4;
#endif

}