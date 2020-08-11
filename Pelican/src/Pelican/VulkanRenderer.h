#pragma once
#include <optional>

#include <vulkan/vulkan.h>

#include "Vertex.h"

namespace Pelican
{
	// Forward declarations
	class Camera;


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
		void SetCamera(Camera* pCamera);

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

		void CreateDescriptorSetLayout();

		void CreateGraphicsPipeline();
		VkShaderModule CreateShaderModule(const std::vector<char>& code);

		void CreateFramebuffers();
		void CreateCommandPool();
		void CreateTextureImage();
		void CreateTextureImageView();
		void CreateTextureSampler();
		void CreateVertexBuffer();
		void CreateIndexBuffer();
		void CreateUniformBuffers();
		void CreateDescriptorPool();
		void CreateDescriptorSets();
		void CreateCommandBuffers();

		void CreateSyncObjects();

		void CleanupSwapChain();
		void RecreateSwapChain();

		uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
		void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
			VkBuffer& buffer, VkDeviceMemory& bufferMemory);
		void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

		VkCommandBuffer BeginSingleTimeCommands();
		void EndSingleTimeCommands(VkCommandBuffer commandBuffer);

		void UpdateUniformBuffer(uint32_t currentImage);

		void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
			VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
		void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
		void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
		VkImageView CreateImageView(VkImage image, VkFormat format);

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
		VkDescriptorSetLayout m_VkDescriptorSetLayout;
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

		VkBuffer m_VkVertexBuffer;
		VkDeviceMemory m_VkVertexBufferMemory;
		VkBuffer m_VkIndexBuffer;
		VkDeviceMemory m_VkIndexBufferMemory;

		// const std::vector<Vertex> m_Vertices = {
		// 	// Left face
		// 	{{-5.0f, -5.0f, -5.0f}, {1.0f, 1.0f, 1.0f}},
		// 	{{ 5.0f, -5.0f, -5.0f}, {1.0f, 1.0f, 1.0f}},
		// 	{{ 5.0f,  5.0f, -5.0f}, {1.0f, 1.0f, 1.0f}},
		// 	{{-5.0f,  5.0f, -5.0f}, {1.0f, 1.0f, 1.0f}},
		//
		// 	// Right face
		// 	{{-5.0f, -5.0f,  5.0f}, {1.0f, 1.0f, 0.0f}},
		// 	{{-5.0f,  5.0f,  5.0f}, {1.0f, 1.0f, 0.0f}},
		// 	{{ 5.0f,  5.0f,  5.0f}, {1.0f, 1.0f, 0.0f}},
		// 	{{ 5.0f, -5.0f,  5.0f}, {1.0f, 1.0f, 0.0f}},
		//
		// 	// Top face
		// 	{{-5.0f,  5.0f, -5.0f}, {0.0f, 1.0f, 0.0f}},
		// 	{{ 5.0f,  5.0f, -5.0f}, {0.0f, 1.0f, 0.0f}},
		// 	{{ 5.0f,  5.0f,  5.0f}, {0.0f, 1.0f, 0.0f}},
		// 	{{-5.0f,  5.0f,  5.0f}, {0.0f, 1.0f, 0.0f}},
		//
		// 	// Bottom face
		// 	{{-5.0f, -5.0f, -5.0f}, {0.0f, 0.0f, 1.0f}},
		// 	{{-5.0f, -5.0f,  5.0f}, {0.0f, 0.0f, 1.0f}},
		// 	{{ 5.0f, -5.0f,  5.0f}, {0.0f, 0.0f, 1.0f}},
		// 	{{ 5.0f, -5.0f, -5.0f}, {0.0f, 0.0f, 1.0f}},
		//
		// 	// Front face
		// 	{{-5.0f, -5.0f, -5.0f}, {1.0f, 0.0f, 0.0f}},
		// 	{{-5.0f,  5.0f, -5.0f}, {1.0f, 0.0f, 0.0f}},
		// 	{{-5.0f,  5.0f,  5.0f}, {1.0f, 0.0f, 0.0f}},
		// 	{{-5.0f, -5.0f,  5.0f}, {1.0f, 0.0f, 0.0f}},
		//
		// 	// Back face
		// 	{{ 5.0f, -5.0f, -5.0f}, {1.0f, 0.5f, 0.0f}},
		// 	{{ 5.0f, -5.0f,  5.0f}, {1.0f, 0.5f, 0.0f}},
		// 	{{ 5.0f,  5.0f,  5.0f}, {1.0f, 0.5f, 0.0f}},
		// 	{{ 5.0f,  5.0f, -5.0f}, {1.0f, 0.5f, 0.0f}},
		// };
		// const std::vector<uint16_t> m_Indices = {
		// 	0, 1, 2, 2, 3, 0,       // Left face
		// 	4, 5, 6, 6, 7, 4,       // Right face
		// 	8, 9, 10, 10, 11, 8,    // Top face
		// 	12, 13, 14, 14, 15, 12, // Bottom face
		// 	16, 17, 18, 18, 19, 16, // Front face
		// 	20, 21, 22, 22, 23, 20, // Back face
		// };

		const std::vector<Vertex> m_Vertices = {
			{{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
			{{-0.5f, -0.5f,  0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
			{{ 0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
			{{ 0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
		};

		const std::vector<uint16_t> m_Indices = {
			0, 1, 2, 2, 3, 0
		};

		std::vector<VkBuffer> m_VkUniformBuffers;
		std::vector<VkDeviceMemory> m_VkUniformBuffersMemory;
		VkDescriptorPool m_VkDescriptorPool;
		std::vector<VkDescriptorSet> m_VkDescriptorSets;

		VkImage m_VkTextureImage;
		VkDeviceMemory m_VkTextureImageMemory;
		VkImageView m_vkTextureImageView;
		VkSampler m_vkTextureSampler;
	};
}
