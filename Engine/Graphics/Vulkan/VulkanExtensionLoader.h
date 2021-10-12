#pragma once
#include "VulkanCommonHeaders.h"

namespace Havana::Graphics::Vulkan
{
#define GET_INSTANCE_PROC_ADDR(inst, entry)												\
	{																					\
		fp##entry = (PFN_vk##entry)vkGetInstanceProcAddr(inst, "vk"#entry);				\
		if (!fp##entry)																	\
			throw std::runtime_error("vkGetInstanceProcAddr failed to find vk"#entry);	\
	}

#define GET_DEVICE_PROC_ADDR(dev, entry)											\
	{																				\
		fp##entry = (PFN_vk##entry)vkGetDeviceProcAddr(dev, "vk"#entry);			\
		if (!fp##entry)																\
			throw std::runtime_error("vkGetDeviceProcAddr failed to find vk"#entry);\
	}

}