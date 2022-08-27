#pragma once
#include "VulkanCommonHeaders.h"
#include "VulkanResources.h"
#include "VulkanExtensionLoader.h"

namespace havana::Graphics::Vulkan
{
	// Indices (locations) of Queue Families (if they exist at all)
	struct QueueFamilyIndices
	{
		int graphicsFamily = -1;			// Location of Graphics Queue Family
		int presentationFamily = -1;		// Location of Presentation Queue Family

		// Check if queue families are valid
		bool is_valid() { return graphicsFamily >= 0 && presentationFamily >= 0; }
	};

	struct SwapChainDetails
	{
		VkSurfaceCapabilitiesKHR surfaceCapabilities;			// Surface properties, e.g. image size/extent
		utl::vector<VkSurfaceFormatKHR> formats;				// Surface image formats supported, e.g. RGBA, size of each color
		utl::vector<VkPresentModeKHR> presentationModes;		// How images should be presented to screen
	};

	struct SwapchainImage
	{
		VkImage image;
		VkImageView imageView;
	};

	struct VulkanDevice
	{
		VkPhysicalDevice physicalDevice;
		VkDevice logicalDevice;
	};
	
	class VulkanSurface
	{
	public:
		explicit VulkanSurface(Platform::Window window) : m_window{ window }
		{
			assert(m_window.Handle());
		}
#if USE_STL_VECTOR // TODO: This has been cpoied over from D3D12Surface and needs to be altered
		DISABLE_COPY(VulkanSurface);
		constexpr VulkanSurface(VulkanSurface&& o)
			: m_swapChain{ o.m_swapChain }, m_window{ o.m_window }, m_currentBBIndex{ o.m_currentBBIndex },
			m_viewport{ o.m_viewport }, m_scissorRect{ o.m_scissorRect }, m_allowTearing{ o.m_allowTearing },
			m_presentFlags{ o.m_presentFlags }
		{
			for (u32 i{ 0 }; i < frameBufferCount; i++)
			{
				m_renderTargetData[i].resource = o.m_renderTargetData[i].resource;
				m_renderTargetData[i].rtv = o.m_renderTargetData[i].rtv;
			}

			o.Reset();
		}

		constexpr VulkanSurface& operator=(VulkanSurface&& o)
		{
			assert(this != &o);
			if (this != &o)
			{
				Release();
				Move(o);
			}

			return *this;
		}
#else
		DISABLE_COPY_AND_MOVE(VulkanSurface);
#endif // USE_STL_VECTOR
		~VulkanSurface() { Release(); }

		void Create(VkInstance instance);
		void Present();
		void Resize();
		constexpr u32 Width() const { return (u32)m_viewport.width; }
		constexpr u32 Height() const { return (u32)m_viewport.height; }
		constexpr const VkViewport& Viewport() const { return m_viewport; }
		constexpr const VkRect2D& ScissorRect() const { return m_scissorRect; }
	private:
		Platform::Window	m_window{};
		VkViewport			m_viewport{};
		VkRect2D			m_scissorRect{};
		int					m_currentFrame{ 0 };
		const u32			m_maxFrameDraws{ 2 }; // this should ideally be one less than the amount of framebuffers
		bool				m_framebufferResized{ false };
		// State
		// Vulkan components
		// - Main
		
		VkInstance m_instance{ nullptr };
		//VkDebugUtilsMessengerEXT m_debugMessanger{ 0 };
		VulkanDevice m_mainDevice;

		VkQueue m_graphicsQueue;
		VkQueue m_presentationQueue;
		VkSurfaceKHR m_surface;
		VkSwapchainKHR m_swapchain;
		utl::vector<SwapchainImage> m_swapchainImages;
		utl::vector<VkFramebuffer>m_swapchainFramebuffers;
		utl::vector<VkCommandBuffer>m_commandBuffers;
		// - Pipeline
		VkPipeline m_graphicsPipeline;
		VkPipelineLayout m_pipelineLayout;
		VkRenderPass m_renderPass;
		VkShaderModule m_vertexShaderModule;
		VkShaderModule m_fragmentShaderModule;
		// - Pools
		VkCommandPool m_graphicsCommandPool;
		// - Utility
		VkFormat m_swapChainImageFormat;
		VkExtent2D m_swapChainExtent;
		// - Syncronisation
		utl::vector<VkSemaphore> m_imageAvailable;
		utl::vector<VkSemaphore> m_renderFinished;
		utl::vector<VkFence> m_drawFences;


		void CreateLogicalDevice();
		void CreateSurface();
		void CreateSwapChain();
		void CreateRenderPass();
		void CreateGraphicsPipeline();
		void CreateFramebuffers();
		void CreateCommandPool();
		void CreateCommandBuffers();
		void CreateSynchronization();
		// - Record Functions
		void RecordCommands();
		// - Get functions
		void GetPhysicalDevice();
		void GetVulkanExtensions();
		// - Support functions
		// -- Checker functions
		bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
		bool CheckDeviceSuitable(VkPhysicalDevice device);
		// -- Getter functions
		QueueFamilyIndices GetQueueFamilies(VkPhysicalDevice device);
		SwapChainDetails GetSwapChainDetails(VkPhysicalDevice device);
		// -- Choose functions
		VkSurfaceFormatKHR ChooseBestSurfaceFormat(const utl::vector<VkSurfaceFormatKHR>& formats);
		VkPresentModeKHR ChooseBestPresentationMode(const utl::vector<VkPresentModeKHR> presentationModes);
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& surfaceCapabilities, int width, int height);
		// -- Create functions
		VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
		VkShaderModule CreateShaderModule(const utl::vector<char>& code);

		void RecreateSwapChain();
		void CleanupSwapChain();

		void Finalize();
		void Release();

		// Function Pointers
		PFN_vkGetPhysicalDeviceSurfaceSupportKHR fpGetPhysicalDeviceSurfaceSupportKHR;
		PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR fpGetPhysicalDeviceSurfaceCapabilitiesKHR;
		PFN_vkGetPhysicalDeviceSurfaceFormatsKHR fpGetPhysicalDeviceSurfaceFormatsKHR;
		PFN_vkGetPhysicalDeviceSurfacePresentModesKHR fpGetPhysicalDeviceSurfacePresentModesKHR;
		PFN_vkCreateSwapchainKHR fpCreateSwapchainKHR;
		PFN_vkDestroySwapchainKHR fpDestroySwapchainKHR;
		PFN_vkGetSwapchainImagesKHR fpGetSwapchainImagesKHR;
		PFN_vkAcquireNextImageKHR fpAcquireNextImageKHR;
		PFN_vkQueuePresentKHR fpQueuePresentKHR;

#if USE_STL_VECTOR // TODO: This has been cpoied over from D3D12Surface and needs to be altered
		constexpr void Reset()
		{
			m_window = {};
			m_swapChain = nullptr;
			for (u32 i{ 0 }; i < bufferCount; i++)
			{
				m_renderTargetData[i] = {};
			}
			m_currentBBIndex = 0;
			m_allowTearing = 0;
			m_presentFlags = 0;
			m_viewport = {};
			m_scissorRect = {};
		}

		constexpr void Move(VulkanSurface& o)
		{
			m_window = o.m_window;
			m_swapChain = o.m_swapChain;
			for (u32 i{ 0 }; i < frameBufferCount; i++)
			{
				m_renderTargetData[i] = o.m_renderTargetData[i];
			}
			m_currentBBIndex = o.m_currentBBIndex;
			m_allowTearing = o.m_allowTearing;
			m_presentFlags = o.m_presentFlags;
			m_viewport = o.m_viewport;
			m_scissorRect = o.m_scissorRect;

			o.Reset();
		}
#endif // USE_STL_VECTOR
	};
}