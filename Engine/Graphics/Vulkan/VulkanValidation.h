#pragma once
#include <iostream>
#include "VulkanCommonHeaders.h"

namespace havana::Graphics::Vulkan
{
	// List of validation layers to use
	// VK_LAYER_KHRONOS_validation = All standard validation layers
	const utl::vector<const char*> validationLayers { 1, "VK_LAYER_KHRONOS_validation" };

	// Only enable validation layers if in debug mode
#ifdef NDEBUG
	constexpr bool enableValidationLayers{ false };
#else
	constexpr bool enableValidationLayers{ true };
#endif

	bool CheckValidationLayerSupport()
	{
		uint32_t layerCount{ 0 };
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		utl::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const auto& layerName : validationLayers)
		{
			bool layerFound{ false };
			for (const auto& layerProperties : availableLayers)
			{
				if (strcmp(layerName, layerProperties.layerName) == 0)
				{
					layerFound = true;
					break;
				}
			}

			if (!layerFound) return false;
		}

		return true;
	}

	// Callback function for validation debugging (will be called when validation information record)
	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{
		if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		{
			// Message is important enough to show
			std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;
		}

		// VK_FALSE indicates that the program should not terminate
		return VK_FALSE;
	}

	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
	{
		// vkGetInstanceProcAddr returns a function pointer to the requested function in the requested instance
		// Resulting function is cast as a function pointer with the header of "vkCreateDebugUtilsMessengerEXT"
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		// If function was found, executre if with given data and return result, otherwise, return error
		if (func != nullptr)
		{
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else
		{
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
	{
		// Get function pointer to requested function, then cast to function pointer for vkDestroyDebugUtilsMessengerEXT
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		// If function found, execute
		if (func != nullptr)
		{
			func(instance, debugMessenger, pAllocator);
		}
	}
}