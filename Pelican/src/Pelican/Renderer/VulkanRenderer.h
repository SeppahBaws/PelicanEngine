#pragma once

#include <vulkan/vulkan.h>

#include "VulkanDevice.h"
#include "VulkanSwapChain.h"

namespace Pelican
{
	// Forward declarations
	class Camera;
	class ImGuiWrapper;

	class VulkanRenderer final
	{
	public:
		VulkanRenderer();

		void Initialize();
		// ImGuiInitInfo GetImGuiInitInfo();
		void BeforeSceneCleanup();
		void AfterSceneCleanup();

		void BeginScene();
		void EndScene();

		void FlagWindowResized() { m_FrameBufferResized = true; }
		void SetCamera(Camera* pCamera);

	public:
		static int GetMaxImages() { return m_pInstance->MAX_FRAMES_IN_FLIGHT; }
		static VkInstance GetInstance() { return m_pInstance->m_VkInstance; }
		static VulkanDevice* GetVulkanDevice() { return m_pInstance->m_pDevice; }
		static VkDevice GetDevice() { return m_pInstance->m_pDevice->GetDevice(); }
		static VkRenderPass GetRenderPass() { return m_pInstance->m_VkRenderPass; }
		static VkPhysicalDevice GetPhysicalDevice() { return m_pInstance->m_pDevice->GetPhysicalDevice(); }
		static VkQueue GetGraphicsQueue() { return m_pInstance->m_pDevice->GetGraphicsQueue(); }
		static VkDescriptorPool GetDescriptorPool() { return m_pInstance->m_VkDescriptorPool; }
		static VkDescriptorSetLayout& GetDescriptorSetLayout() { return m_pInstance->m_VkDescriptorSetLayout; }
		static VkCommandPool GetCommandPool() { return m_pInstance->m_VkCommandPool; }
		static VkCommandBuffer GetCurrentBuffer() { return m_pInstance->m_VkCommandBuffers[m_pInstance->m_CurrentBuffer]; }
		static VkPipelineLayout GetPipelineLayout() { return m_pInstance->m_VkPipelineLayout; }

	private:
		void CreateInstance();
		bool CheckValidationLayerSupport();
		std::vector<const char*> GetRequiredExtensions();
		void PrintExtensions();

		void SetupDebugMessenger();
		void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

		void CreateRenderPass();

		void CreateDescriptorSetLayout();

		void CreateGraphicsPipeline();

		void CreateFramebuffers();
		void CreateCommandPool();
		void CreateDepthResources();
		void CreateUniformBuffers();
		void CreateDescriptorPool();
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
		VulkanSwapChain* m_pSwapChain{};

		VkRenderPass m_VkRenderPass;
		VkDescriptorSetLayout m_VkDescriptorSetLayout;
		VkPipelineLayout m_VkPipelineLayout;
		VkPipeline m_VkGraphicsPipeline;
		std::vector<VkFramebuffer> m_VkSwapChainFramebuffers; // TODO: move this in VulkanSwapChain as well.
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

		VkImage m_VkDepthImage;
		VkDeviceMemory m_VkDepthImageMemory;
		VkImageView m_VkDepthImageView;



		// ImGui
		ImGuiWrapper* m_pImGui{};
	};
}
