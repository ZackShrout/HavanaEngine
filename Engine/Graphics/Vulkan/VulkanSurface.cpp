#include <iostream>
#include <array>
#include <set>
#include "VulkanSurface.h"
#include "VulkanCore.h"

namespace Havana::Graphics::Vulkan
{
	namespace
	{
		const Utils::vector<const char*> deviceExtensions{ 1, VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	} // anonymous namespace
	
	// PUBLIC
	void VulkanSurface::Create(VkInstance instance)
	{
		m_instance = instance;

		try
		{
			CreateSurface();
			GetPhysicalDevice();
			CreateLogicalDevice();
			GetVulkanExtensions();
			CreateSwapChain();
			CreateRenderPass();
			CreateGraphicsPipeline();
			CreateFramebuffers();
			CreateCommandPool();
			CreateCommandBuffers();
			RecordCommands();
			CreateSynchronization();
		}
		catch (const std::runtime_error& e)
		{
			std::cout << "Error: " << e.what() << std::endl;
			assert(false);
			return;
		}
	}

	void VulkanSurface::Present() const
	{
		// Await the fence to signal open
		vkWaitForFences(m_mainDevice.logicalDevice, 1, &m_drawFences[m_currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());
		// Signal fence closed
		vkResetFences(m_mainDevice.logicalDevice, 1, &m_drawFences[m_currentFrame]);

		// ******************** //
		// -- Get next image -- //
		// ******************** //
		uint32_t imageIndex;
		vkAcquireNextImageKHR(m_mainDevice.logicalDevice, m_swapchain, std::numeric_limits<uint64_t>::max(), m_imageAvailable[m_currentFrame], 
			VK_NULL_HANDLE, &imageIndex);

		// ************************************* //
		// -- Submit Command Buffer to render -- //
		// ************************************* //
		VkPipelineStageFlags waitStages[]
		{
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
		};

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &m_imageAvailable[m_currentFrame];
		submitInfo.pWaitDstStageMask = waitStages;			// Stages to check semaphores
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_commandBuffers[imageIndex];
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &m_renderFinished[m_currentFrame];

		// Submit command buffer to queue
		VkCall(vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_drawFences[m_currentFrame]), "Failed to submit command buffer to queue!");

		// ***************************** //
		// -- Present image to screen -- //
		// ***************************** //
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &m_renderFinished[m_currentFrame];
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &m_swapchain;
		presentInfo.pImageIndices = &imageIndex;

		// Present image to screen
		if (vkQueuePresentKHR(m_presentationQueue, &presentInfo) != VK_SUCCESS) throw std::runtime_error("Failed to present queue to screen!");

		// Step to next frame
		m_currentFrame = (m_currentFrame + 1) % maxFrameDraws;
	}

	void VulkanSurface::Resize()
	{
		// TODO: implement
	}

	// PRIVATE
	void VulkanSurface::GetPhysicalDevice()
	{
		// Need to get number of physical devices the instance can access
		uint32_t deviceCount{ 0 };
		assert(vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr) == VK_SUCCESS);
		assert(deviceCount >= 1);

		// Make sure there is at least one device that supports Vulkan
		if (deviceCount == 0)
		{
			throw std::runtime_error("Can't find a GPU that supports Vulkan instance!");
		}

		// Create a list of physical devices the instance can access
		Utils::vector<VkPhysicalDevice> deviceList(deviceCount);
		assert(vkEnumeratePhysicalDevices(m_instance, &deviceCount, deviceList.data()) == VK_SUCCESS);

		// Choose physical device based on it's suitability
		for (const auto& device : deviceList)
		{
			if (CheckDeviceSuitable(device))
			{
				m_mainDevice.physicalDevice = device;
				break;
			}
		}
	}

	void VulkanSurface::GetVulkanExtensions()
	{
		GET_INSTANCE_PROC_ADDR(m_instance, GetPhysicalDeviceSurfaceSupportKHR);
		GET_INSTANCE_PROC_ADDR(m_instance, GetPhysicalDeviceSurfaceCapabilitiesKHR);
		GET_INSTANCE_PROC_ADDR(m_instance, GetPhysicalDeviceSurfaceFormatsKHR);
		GET_INSTANCE_PROC_ADDR(m_instance, GetPhysicalDeviceSurfacePresentModesKHR);
		GET_DEVICE_PROC_ADDR(m_mainDevice.logicalDevice, CreateSwapchainKHR);
		GET_DEVICE_PROC_ADDR(m_mainDevice.logicalDevice, DestroySwapchainKHR);
		GET_DEVICE_PROC_ADDR(m_mainDevice.logicalDevice, GetSwapchainImagesKHR);
		GET_DEVICE_PROC_ADDR(m_mainDevice.logicalDevice, AcquireNextImageKHR);
		GET_DEVICE_PROC_ADDR(m_mainDevice.logicalDevice, QueuePresentKHR);
	}

	bool VulkanSurface::CheckDeviceExtensionSupport(VkPhysicalDevice device)
	{
		// Need to get number of extensions to create array of correct size to hold extensions
		uint32_t extensionCount{ 0 };
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		// No need to go further if there are no extensions supported
		if (extensionCount == 0) return false;

		// Create a list of vkExtensionProperties using extensionCount
		Utils::vector<VkExtensionProperties> extensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, extensions.data());

		// Check if given extensions are in the list of available extensions
		for (const auto& deviceExtension : deviceExtensions)
		{
			bool hasExtension = false;
			for (const auto& extension : extensions)
			{
				if (strcmp(deviceExtension, extension.extensionName) == 0)
				{
					hasExtension = true;
					break;
				}
			}

			if (!hasExtension) return false;
		}

		return true;
	}

	bool VulkanSurface::CheckDeviceSuitable(VkPhysicalDevice device)
	{
		QueueFamilyIndices indices{ GetQueueFamilies(device) };

		bool extensionsSupported{ CheckDeviceExtensionSupport(device) };

		bool swapChainValid{ false };
		if (extensionsSupported)
		{
			SwapChainDetails swapChainDetails = GetSwapChainDetails(device);
			swapChainValid = !swapChainDetails.formats.empty() && !swapChainDetails.presentationModes.empty();
		}

		return indices.IsValid() && extensionsSupported && swapChainValid;
	}

	QueueFamilyIndices VulkanSurface::GetQueueFamilies(VkPhysicalDevice device)
	{
		QueueFamilyIndices indices;

		// Get all Queue Family Property info for the given device
		uint32_t queueFamilyCount{ 0 };
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		Utils::vector<VkQueueFamilyProperties> queueFamilyList(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilyList.data());

		// Go through each queue familty and check if it has at least one of the required tupe of queue
		int i{ 0 };
		for (const auto& queueFamily : queueFamilyList)
		{
			// First check if queue family has at least one queue in that family (could have none)
			// Queue can be multiple types defines through bitfield. Here we check if the graphics bit is set.
			if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				indices.graphicsFamily = i;		// If queue family is valid, then get index
			}

			// Check if queue family supports presentation
			VkBool32 presentationSupport{ false };
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface, &presentationSupport);
			// Check if queue is presentation type (can be both graphics and presentation
			if (queueFamily.queueCount > 0 && presentationSupport)
			{
				indices.presentationFamily = i;
			}

			if (indices.IsValid()) break;		// If queue family indices are in a valid state, break loop

			i++;
		}

		return indices;
	}

	SwapChainDetails VulkanSurface::GetSwapChainDetails(VkPhysicalDevice device)
	{
		SwapChainDetails swapChainDetails{};

		// -- Capabilities --
		// Get the surface capabilities for the given surface on the given physical device
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, &swapChainDetails.surfaceCapabilities);

		// -- Formats --
		uint32_t formatCount{ 0 };
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, nullptr);
		// If formats returned, get list of formats
		if (formatCount != 0)
		{
			swapChainDetails.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, swapChainDetails.formats.data());
		}

		// -- Presentation Modes --
		uint32_t presentationCount{ 0 };
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentationCount, nullptr);
		// If presentation modes returned, get list of presentation modes
		if (presentationCount != 0)
		{
			swapChainDetails.presentationModes.resize(presentationCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentationCount, swapChainDetails.presentationModes.data());
		}

		return swapChainDetails;
	}

	VkSurfaceFormatKHR VulkanSurface::ChooseBestSurfaceFormat(const Utils::vector<VkSurfaceFormatKHR>& formats)
	{
		// VK_FORMAT_UNDEFINED means all formats are available
		if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED)
		{
			return { VK_FORMAT_R8G8B8A8_UNORM , VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
		}

		// If restricted, search for optimal format
		for (const auto& format : formats)
		{
			// We'd be OK with either RBGA or BGRA
			if ((format.format == VK_FORMAT_R8G8B8A8_UNORM || format.format == VK_FORMAT_B8G8R8A8_UNORM) && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				return format;
			}
		}

		// If optimal format is not found, just return first format available
		return formats[0];
	}

	VkPresentModeKHR VulkanSurface::ChooseBestPresentationMode(const Utils::vector<VkPresentModeKHR> presentationModes)
	{
		for (const auto& presentationMode : presentationModes)
		{
			if (presentationMode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				return presentationMode;
			}
		}

		// This is safe because, as a part of the Vulkan spec, this mode must be available.
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D VulkanSurface::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& surfaceCapabilities, int width, int height)
	{
		// If currentExtent is at numeric limits, extent can vary. Otherwise, it is the size of the window.
		if (surfaceCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		{
			return surfaceCapabilities.currentExtent;
		}
		else // If value can vary, it needs to be set manually
		{
			// Create new extent using window size
			VkExtent2D newExtent{};
			newExtent.width = static_cast<uint32_t>(width);
			newExtent.height = static_cast<uint32_t>(height);

			// Make sure extent fits within bounderies of surface max and min
			newExtent.width = std::max(surfaceCapabilities.minImageExtent.width, std::min(surfaceCapabilities.maxImageExtent.width, newExtent.width));
			newExtent.height = std::max(surfaceCapabilities.minImageExtent.height, std::min(surfaceCapabilities.maxImageExtent.height, newExtent.height));

			return newExtent;
		}
	}

	VkImageView VulkanSurface::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
	{
		VkImageViewCreateInfo viewCreateInfo{};
		viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewCreateInfo.image = image;										// Image to create view for
		viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;					// Type of image (1D, 2D, 3D, cube, etc...
		viewCreateInfo.format = format;										// Format of image data
		viewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;		// Allows remapping of rgba components to other rgba values
		viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		// Subresources allow the view to view only a part of the image
		viewCreateInfo.subresourceRange.aspectMask = aspectFlags;			// Which aspect of image to view (i.e. COLOR_BI for viewing color)
		viewCreateInfo.subresourceRange.baseMipLevel = 0;					// Start mipmap level to view from
		viewCreateInfo.subresourceRange.levelCount = 1;						// Number of mipmap levels to vew
		viewCreateInfo.subresourceRange.baseArrayLayer = 0;					// Start array level to view from
		viewCreateInfo.subresourceRange.layerCount = 1;						// Number of array levels to view

		// Create image view and return it
		VkImageView imageView;
		VkCall(vkCreateImageView(m_mainDevice.logicalDevice, &viewCreateInfo, nullptr, &imageView), "Failed to create an Image View!");

		return imageView;
	}

	VkShaderModule VulkanSurface::CreateShaderModule(const Utils::vector<char>& code)
	{
		// Shader Module createion information
		VkShaderModuleCreateInfo shaderModuleCreateInfo{};
		shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shaderModuleCreateInfo.codeSize = code.size();										// Size of code
		shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());		// Pointer to code (of uint32_t pointer type)

		VkShaderModule shaderModule;
		VkCall(vkCreateShaderModule(m_mainDevice.logicalDevice, &shaderModuleCreateInfo, nullptr, &shaderModule), "Failed to create an Shader Module!");

		return shaderModule;
	}

	void VulkanSurface::Finalize()
	{
		// TODO:
	}

	void VulkanSurface::Release()
	{
		// Wait until no action pm device and then destroy everything
		vkDeviceWaitIdle(m_mainDevice.logicalDevice);

		for (size_t i{ 0 }; i < maxFrameDraws; i++)
		{
			vkDestroySemaphore(m_mainDevice.logicalDevice, m_renderFinished[i], nullptr);
			vkDestroySemaphore(m_mainDevice.logicalDevice, m_imageAvailable[i], nullptr);
			vkDestroyFence(m_mainDevice.logicalDevice, m_drawFences[i], nullptr);
		}
		vkDestroyCommandPool(m_mainDevice.logicalDevice, m_graphicsCommandPool, nullptr);
		for (const auto& framebuffer : m_swapchainFramebuffers)
		{
			vkDestroyFramebuffer(m_mainDevice.logicalDevice, framebuffer, nullptr);
		}
		vkDestroyPipeline(m_mainDevice.logicalDevice, m_graphicsPipeline, nullptr);
		vkDestroyPipelineLayout(m_mainDevice.logicalDevice, m_pipelineLayout, nullptr);
		vkDestroyRenderPass(m_mainDevice.logicalDevice, m_renderPass, nullptr);
		for (const auto& image : m_swapchainImages)
		{
			vkDestroyImageView(m_mainDevice.logicalDevice, image.imageView, nullptr);
		}
		vkDestroySwapchainKHR(m_mainDevice.logicalDevice, m_swapchain, nullptr);
		vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
		vkDestroyDevice(m_mainDevice.logicalDevice, nullptr);
	}

	void VulkanSurface::CreateLogicalDevice()
	{
		// Get queue family indices for the chosen Physical Device
		QueueFamilyIndices indices{ GetQueueFamilies(m_mainDevice.physicalDevice) };

		// Vector for queue creation information, and set for family indices
		Utils::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};
		std::set<int> queueFamilyIndices{ indices.graphicsFamily, indices.presentationFamily };

		// Queues the logical device needs to create and info to do so
		for (int queueFamilyIndex : queueFamilyIndices)
		{
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamilyIndex;				// the index of the family to create a queue from
			queueCreateInfo.queueCount = 1;										// Number of queues to create
			float priority{ 1.0f };
			queueCreateInfo.pQueuePriorities = &priority;						// Vulkan needs to know how to handle multiple queues (1 highest priority, 0 lowest)

			queueCreateInfos.push_back(queueCreateInfo);
		}

		// Information to create logical device (sometimes called "device" for short)
		VkDeviceCreateInfo deviceCreateInfo{};
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());		// number of queue create infos
		deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();								// List of queue create infos so device can create required queues
		deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());	// NUmber of enabled logical device extensions
		deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();							// List of enabled logical device extensions

		// Physical device features the logical device will be using
		VkPhysicalDeviceFeatures deviceFeatures{};

		deviceCreateInfo.pEnabledFeatures = &deviceFeatures;					// Physical device features logical device will use

		VkCall(vkCreateDevice(m_mainDevice.physicalDevice, &deviceCreateInfo, nullptr, &m_mainDevice.logicalDevice), "Failed to create a logical device!");

		// Queues are created at the same time as the device so we want a handle to queues
		// From given logical device, of the given index (0 since only 1 queue) of given queue family, we get the VkQueue
		vkGetDeviceQueue(m_mainDevice.logicalDevice, indices.graphicsFamily, 0, &m_graphicsQueue);
		vkGetDeviceQueue(m_mainDevice.logicalDevice, indices.presentationFamily, 0, &m_presentationQueue);
	}

	void VulkanSurface::CreateSurface()
	{
#ifdef _WIN32
		VkWin32SurfaceCreateInfoKHR surfaceCreateInfo{};
		surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		surfaceCreateInfo.hinstance = GetModuleHandle(NULL);
		surfaceCreateInfo.hwnd = (HWND)m_window.Handle();

		VkCall(vkCreateWin32SurfaceKHR(m_instance, &surfaceCreateInfo, nullptr, &m_surface), "Failed to create a surface!");
#elif __linux__
		VkXlibSurfaceCreateInfoKHR surfaceCreateInfo{};
		surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
		surfaceCreateInfo.dpy = (Display*)m_window.Display();
		surfaceCreateInfo.window = (Window)m_window.Handle();

		VkCall(vkCreateXlibSurfaceKHR(m_instance, &surfaceCreateInfo, nullptr, &m_surface), "Failed to create a surface!");
#endif // _WIN32
	}

	void VulkanSurface::CreateSwapChain()
	{
		// Get swap chain details so we can pick best settings
		SwapChainDetails swapChainDetails{ GetSwapChainDetails(m_mainDevice.physicalDevice) };

		// Find optimal surface values for swap chain
		VkSurfaceFormatKHR surfaceFormat = ChooseBestSurfaceFormat(swapChainDetails.formats);
		VkPresentModeKHR presentMode = ChooseBestPresentationMode(swapChainDetails.presentationModes);
		VkExtent2D extent = ChooseSwapExtent(swapChainDetails.surfaceCapabilities, m_window.Width(), m_window.Height());

		// How many images are in teh swap chain? Get one more than the minimum to allow triple buffering, if possible
		uint32_t imageCount = swapChainDetails.surfaceCapabilities.minImageCount + 1;
		// If maxImageCount = 0, there is no max image count limit
		if (swapChainDetails.surfaceCapabilities.maxImageCount > 0 && swapChainDetails.surfaceCapabilities.maxImageCount < imageCount)
		{
			imageCount = swapChainDetails.surfaceCapabilities.maxImageCount;
		}

		// Create information for swap chain
		VkSwapchainCreateInfoKHR swapChainCreateInfo = {};
		swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapChainCreateInfo.surface = m_surface;
		swapChainCreateInfo.imageFormat = surfaceFormat.format;
		swapChainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
		swapChainCreateInfo.presentMode = presentMode;
		swapChainCreateInfo.imageExtent = extent;
		swapChainCreateInfo.minImageCount = imageCount;
		swapChainCreateInfo.imageArrayLayers = 1;													// Number of layers for each image in chain
		swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;						// What attachment images will be used as
		swapChainCreateInfo.preTransform = swapChainDetails.surfaceCapabilities.currentTransform;	// Transform to perform on swapchain images
		swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;						// How to handle blending images with external graphics
		swapChainCreateInfo.clipped = VK_TRUE;														// Clip parts of image not in view? (i.e. off screen, behind another window...)

		// Get queue famiy indices
		QueueFamilyIndices indices = GetQueueFamilies(m_mainDevice.physicalDevice);

		// If graphics & presentation fmailies are different, then swapchain must let images be shared between families
		if (indices.graphicsFamily != indices.presentationFamily)
		{
			u32 queueFamilyIndices[]{
				(u32)indices.graphicsFamily,
				(u32)indices.presentationFamily
			};

			swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;		// Image share handling
			swapChainCreateInfo.queueFamilyIndexCount = 2;							// Number of queues to share images between
			swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;			// Array of queues to share between
		}
		else
		{
			swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;		// Image share handling
			swapChainCreateInfo.queueFamilyIndexCount = 0;							// Number of queues to share images between
			swapChainCreateInfo.pQueueFamilyIndices = nullptr;						// Array of queues to share between
		}

		// If old swap chain has been destroyed and this one replaces it, link old one to quickly hand over responsibilities
		swapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

		// Create Swapchain
		VkCall(vkCreateSwapchainKHR(m_mainDevice.logicalDevice, &swapChainCreateInfo, nullptr, &m_swapchain), "Failed to create Swapchain!");

		// Store for later reference
		m_swapChainImageFormat = surfaceFormat.format;
		m_swapChainExtent = extent;

		// Get swap chain images (first count, then values)
		u32 swapChainImageCount{ 0 };
		vkGetSwapchainImagesKHR(m_mainDevice.logicalDevice, m_swapchain, &swapChainImageCount, nullptr);
		Utils::vector<VkImage> images(swapChainImageCount);
		vkGetSwapchainImagesKHR(m_mainDevice.logicalDevice, m_swapchain, &swapChainImageCount, images.data());

		for (const auto& image : images)
		{
			// Store image handle
			SwapchainImage swapChainImage{};
			swapChainImage.image = image;
			swapChainImage.imageView = CreateImageView(image, m_swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);

			// Add to swap chain image list
			m_swapchainImages.push_back(swapChainImage);
		}
	}

	void VulkanSurface::CreateRenderPass()
	{
		// Color Attachment of render pass
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = m_swapChainImageFormat;					// format to use for attachment
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;					// number of samples to write for multisampling
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;				// what to do with attachment before rendering
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;				// what to do with attachment after rendering
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;	// what to do with stencil before rendering
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;	// what to do with stencil after rendering
		// Framebuffer data will be stored as an image, but images can be given different data layouts to give optimal use for certain operations
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;			// image data layout before render pass starts
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;		// image data layout after render pass (layout to change to)

		// Attachment reference uses an attachment index that refers to index in the attachment list passed to renderPassCreateInfo
		VkAttachmentReference colorAttachmentReference{};
		colorAttachmentReference.attachment = 0;									// index of attachment in render pass to use
		colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;	// image data layout during graphics subpass of the render pass

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;		// pipeline type subpass is to be bound to
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentReference;

		// Need to determine when layout transitions occur using subpass dependencies
		std::array<VkSubpassDependency, 2> subpassDependencies;
		// Conversion from VK_IMAGE_LAYOUT_UNDEFINED to VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		//............... Conversion happens after ................
		subpassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;															// special value meaning outside of renderpass
		subpassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;											// pipeline stage
		subpassDependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;													// stage access mask
		//............... but before ..............................
		subpassDependencies[0].dstSubpass = 0;
		subpassDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		subpassDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		subpassDependencies[0].dependencyFlags = 0;
		// Conversion from VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL to VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
		subpassDependencies[1].srcSubpass = 0;																				// after first subpass
		subpassDependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		subpassDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		subpassDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		subpassDependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		subpassDependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		subpassDependencies[1].dependencyFlags = 0;

		// Create Info for render pass
		VkRenderPassCreateInfo renderPassCreateInfo{};
		renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassCreateInfo.attachmentCount = 1;
		renderPassCreateInfo.pAttachments = &colorAttachment;
		renderPassCreateInfo.subpassCount = 1;
		renderPassCreateInfo.pSubpasses = &subpass;
		renderPassCreateInfo.dependencyCount = static_cast<uint32_t>(subpassDependencies.size());
		renderPassCreateInfo.pDependencies = subpassDependencies.data();

		VkCall(vkCreateRenderPass(m_mainDevice.logicalDevice, &renderPassCreateInfo, nullptr, &m_renderPass), "Failed to create a Render Pass!");
	}

	void VulkanSurface::CreateGraphicsPipeline()
	{
		// Read in SPIR-V shader code
		auto vertexShaderCode{ ReadFile("../../Engine/Graphics/Vulkan/Shaders/vert.spv") };
		auto fragmentShaderCode{ ReadFile("../../Engine/Graphics/Vulkan/Shaders/frag.spv") };

		// Create Shader Modules to link to Graphics Pipeline
		VkShaderModule vertexShaderModule = CreateShaderModule(vertexShaderCode);
		VkShaderModule fragmentShaderModule = CreateShaderModule(fragmentShaderCode);

		// *************************************** //
		// -- Shader Stage Creation Information -- //
		// *************************************** //
		// Vertex Stage creation information
		VkPipelineShaderStageCreateInfo vertexShaderCreateInfo{};
		vertexShaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertexShaderCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;					// Shader stage name
		vertexShaderCreateInfo.module = vertexShaderModule;							// Shader module used by stage
		vertexShaderCreateInfo.pName = "main";										// Entry point in shader

		// Fragment Stage creation information
		VkPipelineShaderStageCreateInfo fragmentShaderCreateInfo{};
		fragmentShaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragmentShaderCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;					// Shader stage name
		fragmentShaderCreateInfo.module = fragmentShaderModule;							// Shader module used by stage
		fragmentShaderCreateInfo.pName = "main";										// Entry point in shader

		// Array pf shader stage create infos for the creation of the graphics pipeline
		VkPipelineShaderStageCreateInfo shaderStages[]{ vertexShaderCreateInfo, fragmentShaderCreateInfo };

		// ********************************************* //
		// -- Vertex Input State Creation Information -- //
		// ********************************************* //
		// TODO: Put in vertex descriptions when resources created
		VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo{};
		vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputCreateInfo.vertexBindingDescriptionCount = 0;
		vertexInputCreateInfo.pVertexAttributeDescriptions = nullptr;				// List of Vertex Binding Descriptions (data spacing/stride info/etc...)
		vertexInputCreateInfo.vertexAttributeDescriptionCount = 0;
		vertexInputCreateInfo.pVertexAttributeDescriptions = nullptr;				// List of Vertex Attribute Descriptions (data format and hwere to bind to/from)

		// *********************************************** //
		// -- Input Assembly State Creation Information -- //
		// *********************************************** //
		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;				// Primitive type to assemble vertices in
		inputAssembly.primitiveRestartEnable = VK_FALSE;							// Allow overriding of "strip" topology to start new primitives

		// *************************************************** //
		// -- Viewport & Scissor State Creation Information -- //
		// *************************************************** //
		// Create a viewport info struct
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(m_swapChainExtent.width);
		viewport.height = static_cast<float>(m_swapChainExtent.height);
		viewport.minDepth = 0.0;											// min framebuffer depth
		viewport.maxDepth = 1.0f;											// max framebuffer depth
		m_viewport = viewport;

		// Create a scissor info struct
		VkRect2D scissor{};
		scissor.offset = { 0, 0 };											// offset to use region from
		scissor.extent = { m_swapChainExtent };								// extent to describe region to use, starting at offset
		m_scissorRect = scissor;

		VkPipelineViewportStateCreateInfo viewportStateCreateInfo{};
		viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportStateCreateInfo.viewportCount = 1;
		viewportStateCreateInfo.pViewports = &viewport;
		viewportStateCreateInfo.scissorCount = 1;
		viewportStateCreateInfo.pScissors = &scissor;

		// **************************************** //
		// -- Dynamic State Creation Information -- //     --> uncomment to use
		// **************************************** //
		// Dynamic states to enable
		//std::vector<VkDynamicState> dynamicStateEnables;
		//dynamicStateEnables.push_back(VK_DYNAMIC_STATE_VIEWPORT);	// makes the viewport and scissor dynamic so it can be resized in the command buffer
		//dynamicStateEnables.push_back(VK_DYNAMIC_STATE_SCISSOR);	// with vkCmdSetViewport() and vkCmdSetScissor(), etc...

		//VkPipelineDynamicStateCreateInfo dynamicStateInfo{};
		//dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		//dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());
		//dynamicStateInfo.pDynamicStates = dynamicStateEnables.data();

		// ******************************************* //
		// -- Rasterizer State Creation Information -- //
		// ******************************************* //
		VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo{};
		rasterizerCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizerCreateInfo.depthClampEnable = VK_FALSE;			// enable to flatten everything beyond the far plane to the far plane to avoid clipping - but needs the depthClamp device feature
		rasterizerCreateInfo.rasterizerDiscardEnable = VK_FALSE;	// only enabled for gpu calculations
		rasterizerCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;	// How to handle filling points between vertices - anything other than fill needs a gpu feature
		rasterizerCreateInfo.lineWidth = 1.0f;						// How thick lines should be drawn - anything other than 1.0f requires a gpu feature
		rasterizerCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;		// Which face (if any) to cull
		rasterizerCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;	// which winding to use, which defines our front face.
		rasterizerCreateInfo.depthBiasEnable = VK_FALSE;			// whether to add depth bias to fragments - good for stopping shadow acne on shadow map

		// ********************************************** //
		// -- Multisampling State Creation Information -- //
		// ********************************************** //
		VkPipelineMultisampleStateCreateInfo multisamplingCreateInfo{};
		multisamplingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisamplingCreateInfo.sampleShadingEnable = VK_FALSE;					// enable multisample shading or not
		multisamplingCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;	// number of samples to use per fragment

		// ***************************************** //
		// -- Blending State Creation Information -- //
		// ***************************************** //
		VkPipelineColorBlendAttachmentState colorState{};
		colorState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorState.blendEnable = VK_TRUE;
		// Blending equiation: (srcColorBlendFactor * new color) colorBlendOp (dstBlendFactor * old color)
		colorState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colorState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorState.colorBlendOp = VK_BLEND_OP_ADD;
		colorState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;		//
		colorState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;		// --> replaces old alpha with new alpha... no blending
		colorState.alphaBlendOp = VK_BLEND_OP_ADD;					//

		VkPipelineColorBlendStateCreateInfo colorBlendingCreateInfo{};
		colorBlendingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendingCreateInfo.logicOpEnable = VK_FALSE;					// alternative to calculations is to use logical operations
		colorBlendingCreateInfo.attachmentCount = 1;
		colorBlendingCreateInfo.pAttachments = &colorState;

		// ****************************************** //
		// -- Pipeline Layout Creation Information -- //
		// ****************************************** //
		// TODO: Apply future Descriptor Set Layouts
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
		pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCreateInfo.setLayoutCount = 0;
		pipelineLayoutCreateInfo.pSetLayouts = nullptr;
		pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
		pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

		VkCall(vkCreatePipelineLayout(m_mainDevice.logicalDevice, &pipelineLayoutCreateInfo, nullptr, &m_pipelineLayout), "Failed to create Pipeline Layout!");

		// *************************** //
		// -- Depth Stencil Testing -- //
		// *************************** //
		// TODO: set up depth stencil testing

		// ******************************************** //
		// -- Graphics Pipeline Creation Information -- //
		// ******************************************** //
		VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
		pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineCreateInfo.stageCount = 2;
		pipelineCreateInfo.pStages = shaderStages;
		pipelineCreateInfo.pVertexInputState = &vertexInputCreateInfo;
		pipelineCreateInfo.pInputAssemblyState = &inputAssembly;
		pipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
		pipelineCreateInfo.pDynamicState = nullptr;
		pipelineCreateInfo.pRasterizationState = &rasterizerCreateInfo;
		pipelineCreateInfo.pMultisampleState = &multisamplingCreateInfo;
		pipelineCreateInfo.pColorBlendState = &colorBlendingCreateInfo;
		pipelineCreateInfo.pDepthStencilState = nullptr;
		pipelineCreateInfo.layout = m_pipelineLayout;
		pipelineCreateInfo.renderPass = m_renderPass;
		pipelineCreateInfo.subpass = 0;
		// Pipeline derivatives: can create multiple pipelines that derive from one another for optimization
		pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;		// Existing pipeine to derive from
		pipelineCreateInfo.basePipelineIndex = -1;					// Or index of pipeline being created to derive from

		VkCall(vkCreateGraphicsPipelines(m_mainDevice.logicalDevice, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &m_graphicsPipeline),
			"Failed to create Graphics Pipeline!");

		// Shader modules not needed after pipeline created
		//vkDestroyShaderModule(m_mainDevice.logicalDevice, fragmentShaderModule, nullptr);
		//vkDestroyShaderModule(m_mainDevice.logicalDevice, vertexShaderModule, nullptr);
	}

	void VulkanSurface::CreateFramebuffers()
	{
		// Resize frame buffer count to equal swapchain image count
		m_swapchainFramebuffers.resize(m_swapchainImages.size());

		// Create a framebuffer for each swap chain image
		for (size_t i{ 0 }; i < m_swapchainFramebuffers.size(); i++)
		{
			std::array<VkImageView, 1> attachments{
				m_swapchainImages[i].imageView
			};

			VkFramebufferCreateInfo framebufferCreateInfo{};
			framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferCreateInfo.renderPass = m_renderPass;									// render pass layout the framebuffer will be used with
			framebufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			framebufferCreateInfo.pAttachments = attachments.data();							// list of attachments - 1:1 with render pass
			framebufferCreateInfo.width = m_swapChainExtent.width;
			framebufferCreateInfo.height = m_swapChainExtent.height;
			framebufferCreateInfo.layers = 1;

			VkCall(vkCreateFramebuffer(m_mainDevice.logicalDevice, &framebufferCreateInfo, nullptr, &m_swapchainFramebuffers[i]),
				"Failed to create a Framebuffer!");
		}
	}

	void VulkanSurface::CreateCommandPool()
	{
		// Get indices of queue families from device
		QueueFamilyIndices queueFamilyIndices = GetQueueFamilies(m_mainDevice.physicalDevice);

		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;		// Queue family type that buffers from this command pool will use

		// Create a graphics queue family command pool
		VkCall(vkCreateCommandPool(m_mainDevice.logicalDevice, &poolInfo, nullptr, &m_graphicsCommandPool), "Failed to creat Graphics Command Pool!");
	}

	void VulkanSurface::CreateCommandBuffers()
	{
		// Resize command buffer count to have one for each framebuffer
		m_commandBuffers.resize(m_swapchainFramebuffers.size());

		VkCommandBufferAllocateInfo commandBufferAllocInfo{};
		commandBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		commandBufferAllocInfo.commandPool = m_graphicsCommandPool;
		commandBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; // primaries are executed by queues, secondaries are executed by primaries
		commandBufferAllocInfo.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());

		// Allocate command buffers and place handles in array of buffers
		VkCall(vkAllocateCommandBuffers(m_mainDevice.logicalDevice, &commandBufferAllocInfo, m_commandBuffers.data()), "Failed to create Command Buffers!");
	}

	void VulkanSurface::CreateSynchronization()
	{
		m_imageAvailable.resize(maxFrameDraws);
		m_renderFinished.resize(maxFrameDraws);
		m_drawFences.resize(maxFrameDraws);

		// Semaphore creation information
		VkSemaphoreCreateInfo semaphoreCreateInfo{};
		semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		// Fence creation information
		VkFenceCreateInfo fenceCreateInfo{};
		fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;			// Be sure fence starts signaled as open

		for (size_t i{ 0 }; i < maxFrameDraws; i++)
		{
			if (vkCreateSemaphore(m_mainDevice.logicalDevice, &semaphoreCreateInfo, nullptr, &m_imageAvailable[i]) != VK_SUCCESS ||
				vkCreateSemaphore(m_mainDevice.logicalDevice, &semaphoreCreateInfo, nullptr, &m_renderFinished[i]) != VK_SUCCESS ||
				vkCreateFence(m_mainDevice.logicalDevice, &fenceCreateInfo, nullptr, &m_drawFences[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to create at least one Semaphore and/or Fence!");
			}
		}
	}

	void VulkanSurface::RecordCommands()
	{
		// Information about how to begin each command buffer
		VkCommandBufferBeginInfo bufferBeginInfo{};
		bufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		// Color to clear framebuffer to
		VkClearValue clearValues[]
		{
			{0.8f, 0.6f, 0.7f, 1.0f}
		};

		// Information about how to begin a render pass (only needed for graphical applications)
		VkRenderPassBeginInfo renderPassBeginInfo{};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass = m_renderPass;								// render pass to begin
		renderPassBeginInfo.renderArea.offset = { 0,0 };							// start point of render pass in pixels
		renderPassBeginInfo.renderArea.extent = m_swapChainExtent;					// size of region to render
		renderPassBeginInfo.clearValueCount = 1;
		renderPassBeginInfo.pClearValues = clearValues;								// TODO: add depth attachment

		for (size_t i{ 0 }; i < m_commandBuffers.size(); i++)
		{
			renderPassBeginInfo.framebuffer = m_swapchainFramebuffers[i];

			// Start recording commands to the command buffer
			VkCall(vkBeginCommandBuffer(m_commandBuffers[i], &bufferBeginInfo), "Failed to start recording a command buffer!");

			// Begin render pass
			vkCmdBeginRenderPass(m_commandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

			// Bind pipeline to be used in render pass
			vkCmdBindPipeline(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);

			// Execute pipeline
			vkCmdDraw(m_commandBuffers[i], 3, 1, 0, 0);

			// End render pass
			vkCmdEndRenderPass(m_commandBuffers[i]);

			// Stop recording commands to the command buffer
			VkCall(vkEndCommandBuffer(m_commandBuffers[i]), "Failed to stop recording a command buffer!");
		}
	}
}