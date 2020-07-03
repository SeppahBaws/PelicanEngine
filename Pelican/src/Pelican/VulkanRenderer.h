#pragma once
#include <optional>

#include <vulkan/vulkan.h>

namespace Pelican
{
	struct QueueFamilyIndices
	{
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool IsComplete() const
		{
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};

	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	class VulkanRenderer final
	{
	public:
		VulkanRenderer() = default;

		void Initialize();
		void Cleanup();

		void Draw();
		void FlagWindowResized() { m_FrameBufferResized = true; }

	private:
		void CreateInstance();
		bool CheckValidationLayerSupport();
		std::vector<const char*> GetRequiredExtensions();
		void PrintExtensions();

		void SetupDebugMessenger();
		void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

		void CreateSurface();

		void PickPhysicalDevice();
		bool IsDeviceSuitable(VkPhysicalDevice device);
		QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
		void CreateLogicalDevice();

		bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

		void CreateSwapChain();
		void CreateImageViews();

		void CreateRenderPass();

		void CreateGraphicsPipeline();
		VkShaderModule CreateShaderModule(const std::vector<char>& code);

		void CreateFramebuffers();
		void CreateCommandPool();
		void CreateCommandBuffers();

		void CreateSyncObjects();

		void CleanupSwapChain();
		void RecreateSwapChain();

	private:
		static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData);

	private:
		static std::vector<char> ReadFile(const std::string& filename);

	private:
		VkInstance m_VkInstance{};
#ifdef PELICAN_DEBUG
		const bool m_EnableValidationLayers = true;
#else
		const bool m_EnableValidationLayers = false;
#endif
		const std::vector<const char*> m_ValidationLayers = {
			"VK_LAYER_KHRONOS_validation"
		};
		VkDebugUtilsMessengerEXT m_VkDebugMessenger{};
		VkPhysicalDevice m_VkPhysicalDevice{};
		VkDevice m_VkDevice{};
		VkQueue m_VkGraphicsQueue;
		VkSurfaceKHR m_VkSurface;
		VkQueue m_VkPresentQueue;
		const std::vector<const char*> deviceExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};
		VkSwapchainKHR m_VkSwapchain;
		std::vector<VkImage> m_VkSwapChainImages;
		VkFormat m_VkSwapChainImageFormat;
		VkExtent2D m_VkSwapChainExtent;
		std::vector<VkImageView> m_VkSwapChainImageViews;
		VkRenderPass m_VkRenderPass;
		VkPipelineLayout m_VkPipelineLayout;
		VkPipeline m_VkGraphicsPipeline;
		std::vector<VkFramebuffer> m_VkSwapChainFramebuffers;
		VkCommandPool m_VkCommandPool;
		std::vector<VkCommandBuffer> m_VkCommandBuffers;
		const int MAX_FRAMES_IN_FLIGHT = 2;
		size_t m_CurrentFrame = 0;
		std::vector<VkSemaphore> m_VkImageAvailableSemaphores;
		std::vector<VkSemaphore> m_VkRenderFinishedSemaphores;
		std::vector<VkFence> m_VkInFlightFences;
		std::vector<VkFence> m_VkImagesInFlight;

		bool m_FrameBufferResized = false;
	};
}
