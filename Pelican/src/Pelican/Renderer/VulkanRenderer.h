#pragma once

#include <vulkan/vulkan.h>

#include "VulkanDevice.h"

namespace Pelican
{
	// Forward declarations
	class Camera;

	class VulkanRenderer final
	{
	public:
		VulkanRenderer();

		void Initialize();
		void BeforeSceneCleanup();
		void AfterSceneCleanup();

		void BeginScene();
		void EndScene();

		void FlagWindowResized() { m_FrameBufferResized = true; }
		void SetCamera(Camera* pCamera);

	public:
		static VkDevice GetDevice() { return m_pInstance->m_pDevice->GetDevice(); }
		static VkPhysicalDevice GetPhysicalDevice() { return m_pInstance->m_pDevice->GetPhysicalDevice(); }
		static VkQueue GetGraphicsQueue() { return m_pInstance->m_pDevice->GetGraphicsQueue(); }
		static VkCommandPool GetCommandPool() { return m_pInstance->m_VkCommandPool; }
		static VkCommandBuffer GetCurrentBuffer() { return m_pInstance->m_VkCommandBuffers[m_pInstance->m_CurrentBuffer]; }

	private:
		void CreateInstance();
		bool CheckValidationLayerSupport();
		std::vector<const char*> GetRequiredExtensions();
		void PrintExtensions();

		void SetupDebugMessenger();
		void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

		void CreateSwapChain();
		void CreateImageViews();

		void CreateRenderPass();

		void CreateDescriptorSetLayout();

		void CreateGraphicsPipeline();
		VkShaderModule CreateShaderModule(const std::vector<char>& code);

		void CreateFramebuffers();
		void CreateCommandPool();
		void CreateDepthResources();
		void CreateTextureImage();
		void CreateTextureImageView();
		void CreateTextureSampler();
		void CreateUniformBuffers();
		void CreateDescriptorPool();
		void CreateDescriptorSets();
		void CreateCommandBuffers();

		void CreateSyncObjects();

		void CleanupSwapChain();
		void RecreateSwapChain();

		void UpdateUniformBuffer(uint32_t currentImage);

		void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
			VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
		void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
		void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
		VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

		VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
		VkFormat FindDepthFormat();
		bool HasStencilComponent(VkFormat format);

		void BeginCommandBuffers();
		void EndCommandBuffers();

	private:
		static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData);

	private:
		static std::vector<char> ReadFile(const std::string& filename);

	private:
		Camera* m_pCamera = nullptr;

	private:
		static VulkanRenderer* m_pInstance;

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

		VulkanDevice* m_pDevice{};

		VkSwapchainKHR m_VkSwapchain;
		std::vector<VkImage> m_VkSwapChainImages;
		VkFormat m_VkSwapChainImageFormat;
		VkExtent2D m_VkSwapChainExtent;
		std::vector<VkImageView> m_VkSwapChainImageViews;
		VkRenderPass m_VkRenderPass;
		VkDescriptorSetLayout m_VkDescriptorSetLayout;
		VkPipelineLayout m_VkPipelineLayout;
		VkPipeline m_VkGraphicsPipeline;
		std::vector<VkFramebuffer> m_VkSwapChainFramebuffers;
		VkCommandPool m_VkCommandPool;
		std::vector<VkCommandBuffer> m_VkCommandBuffers;
		const int MAX_FRAMES_IN_FLIGHT = 2;
		size_t m_CurrentFrame = 0;
		uint32_t m_CurrentBuffer;
		std::vector<VkSemaphore> m_VkImageAvailableSemaphores;
		std::vector<VkSemaphore> m_VkRenderFinishedSemaphores;
		std::vector<VkFence> m_VkInFlightFences;
		std::vector<VkFence> m_VkImagesInFlight;

		bool m_FrameBufferResized = false;

		std::vector<VkBuffer> m_VkUniformBuffers;
		std::vector<VkDeviceMemory> m_VkUniformBuffersMemory;
		VkDescriptorPool m_VkDescriptorPool;
		std::vector<VkDescriptorSet> m_VkDescriptorSets;

		VkImage m_VkTextureImage;
		VkDeviceMemory m_VkTextureImageMemory;
		VkImageView m_vkTextureImageView;
		VkSampler m_vkTextureSampler;

		VkImage m_VkDepthImage;
		VkDeviceMemory m_VkDepthImageMemory;
		VkImageView m_VkDepthImageView;
	};
}
