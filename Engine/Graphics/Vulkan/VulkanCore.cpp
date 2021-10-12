#include <iostream>
#include "VulkanCore.h"
#include "VulkanResources.h"
#include "VulkanSurface.h"
#include "VulkanHelpers.h"
#include "VulkanValidation.h"

namespace Havana::Graphics::Vulkan::Core
{
	namespace
	{
		using surface_collection = Utils::free_list<VulkanSurface>;
		
		surface_collection			surfaces;
		VkInstance					instance{ nullptr };
		VkDebugUtilsMessengerEXT	debugMessanger{ 0 };
		
		bool CheckInstanceExtensionSupport(Utils::vector<const char*>* checkExtensions)
		{
			// Need to get number of extensions to create array of correct size to hold extensions
			uint32_t extensionCount{ 0 };
			vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

			// No need to go further if there are no extensions supported
			if (extensionCount == 0) return false;

			// Create a list of vkExtensionProperties using extensionCount
			Utils::vector<VkExtensionProperties> extensions(extensionCount);
			vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

			// Check if given extensions are in the list of available extensions
			for (const auto& checkExtension : *checkExtensions)
			{
				bool hasExtension = false;
				for (const auto& extension : extensions)
				{
					if (strcmp(checkExtension, extension.extensionName) == 0)
					{
						hasExtension = true;
						break;
					}
				}

				if (!hasExtension) return false;
			}

			return true;
		}

		void CreateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
		{
			createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
			createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
			createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
			createInfo.pfnUserCallback = DebugCallback;
			createInfo.pUserData = nullptr; // optional
		}
		
		void CreateInstance()
		{
			if (enableValidationLayers && !CheckValidationLayerSupport())
			{
				throw std::runtime_error("Validation layers requested, but not available!");
			}

			// Information about the application itself
			// NOTE: Most data here doesn't affect the program and is for develper convenience
			VkApplicationInfo appInfo{};
			appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
			appInfo.pApplicationName = "Application Name";			// Custom name of application
			appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);	// Custom version of application
			appInfo.pEngineName = "Havana";							// Custome name of engine
			appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);		// Custom version of engine
			appInfo.apiVersion = VK_API_VERSION_1_0;				// Version of Vulkan used... this will affect program

			// Creation information for a VkInstance (Vulkan Instance)
			VkInstanceCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
			createInfo.pApplicationInfo = &appInfo;

			// Create list to hold instance extensions
			Utils::vector<const char*> instanceExtensions{ 1, VK_KHR_SURFACE_EXTENSION_NAME };

#ifdef _WIN32
			instanceExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif __linux__
			instanceExtensions.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#endif // _WIN32

			// If validation enabled, add extension to report validation debug info
			if (enableValidationLayers)
			{
				instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			}
			// Check instance extensions supported
			if (!CheckInstanceExtensionSupport(&instanceExtensions))
			{
				throw std::runtime_error("VKInstance does not support required extensions!");
			}

			// Additional debug messenger that will be used during instance creation and instance destruction
			// Note: created outside of if (enableValidationLayers) to make sure it isn't destroyed before the
			//		 vkCreateInstance call.
			VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};

			// Continue filling creation information
			createInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
			createInfo.ppEnabledExtensionNames = instanceExtensions.data();

			if (enableValidationLayers)
			{
				createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
				createInfo.ppEnabledLayerNames = validationLayers.data();

				// Pointer to callback functions to enable validation of instance creation and destruction
				CreateDebugMessengerCreateInfo(debugCreateInfo);
				createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
			}
			else
			{
				createInfo.enabledLayerCount = 0;
				createInfo.ppEnabledLayerNames = nullptr;
				createInfo.pNext = nullptr;
			}

			// Create instance
			VkCall(vkCreateInstance(&createInfo, nullptr, &instance), "Failed to create a Vulkan Instance!");
		}

		void CreateDebugCallback()
		{
			// Only create callback if validation enabled
			if (!enableValidationLayers) return;

			VkDebugUtilsMessengerCreateInfoEXT createInfo;
			CreateDebugMessengerCreateInfo(createInfo);

			VkCall(CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessanger), "Failed to set up debug messenger!");
		}
	} // anonymous namespace
	
	bool Initialize() 
	{ 
		try
		{
			CreateInstance();
			CreateDebugCallback();
		}
		catch (const std::runtime_error& e)
		{
			std::cout << "Error: " << e.what() << std::endl;
			return false;
		}
		
		return true; 
	}
	void Shutdown() 
	{

		if (enableValidationLayers)
		{
			DestroyDebugUtilsMessengerEXT(instance, debugMessanger, nullptr);
		}
		vkDestroyInstance(instance, nullptr);
	}

	u32 CurrentFrameIndex() { return 0; }

	Surface CreateSurface(Platform::Window window) 
	{ 
		surface_id id{ surfaces.add(window) };
		surfaces[id].Create(instance);
		return Surface{ id };
	}
	void RemoveSurface(surface_id id) 
	{
		surfaces.remove(id);
	}
	
	void ResizeSurface(surface_id id, u32, u32) {}
	
	u32 SurfaceWidth(surface_id id) { return 0; }
	
	u32 SurfaceHeight(surface_id id) { return 0; }
	
	void RenderSurface(surface_id id) 
	{
		surfaces[id].Present();
	}
}