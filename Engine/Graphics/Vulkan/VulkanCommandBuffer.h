#pragma once
#include "VulkanCommonHeaders.h"

namespace havana::graphics::vulkan
{
	vulkan_cmd_buffer allocate_cmd_buffer(VkDevice device, VkCommandPool cmd_pool, bool primary);
	void free_cmd_buffer(VkDevice device, VkCommandPool cmd_pool, vulkan_cmd_buffer& cmd_buffer);
	void begin_cmd_buffer(vulkan_cmd_buffer& cmd_buffer, bool single_use, bool renderpass_continue, bool simultaneous_use);
	void end_cmd_buffer(vulkan_cmd_buffer& cmd_buffer);
	void update_cmd_buffer_submitted(vulkan_cmd_buffer& cmd_buffer);
	void reset_cmd_buffer(vulkan_cmd_buffer& cmd_buffer);

	vulkan_cmd_buffer allocate_cmd_buffer_begin_single_use(VkDevice device, VkCommandPool cmd_pool);
	void end_cmd_buffer_single_use(VkDevice device, VkCommandPool cmd_pool, vulkan_cmd_buffer& cmd_buffer, VkQueue queue);
}