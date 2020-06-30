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

	class VulkanDevice final
	{
	public:
		VulkanDevice() = default;

		void Initialize();
		void Cleanup();

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

	private:
		static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData);

	private:
		VkInstance m_VkInstance{};
		const bool m_EnableValidationLayers = true;
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
	};
}
