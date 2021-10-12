#pragma once

#include <vulkan/vulkan.h>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif // NOMINMAX
#ifndef WIN_32_LEAN_AND_MEAN
#define WIN_32_LEAN_AND_MEAN
#endif // WIN_32_LEAN_AND_MEAN
#include <Windows.h>
#include <vulkan/vulkan_win32.h>
#include <wrl.h>
#elif __linux__
#include <X11/Xlib.h>
#include <vulkan/vulkan_xlib.h>
#endif // _WIN32

#include "CommonHeaders.h"
#include "../Graphics/Renderer.h"

namespace Havana::Graphics::Vulkan
{
	constexpr u32 frameBufferCount{ 3 };
}

#ifdef _DEBUG
#ifndef VkCall
#define VkCall(func, msg)						\
if(func != VK_SUCCESS)							\
	throw std::runtime_error(msg)
#endif // !VkCall
#else
#ifndef VkCall
#define VkCall(func, msg) func
#endif // !VkCall
#endif // _DEBUG