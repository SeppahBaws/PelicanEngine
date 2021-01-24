#pragma once

#include <vulkan/vulkan.h>

namespace Pelican
{
	class VulkanDevice;

	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	const std::vector<const char*> g_DeviceExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

#define LOGE(...) fprintf(stderr, "ERROR: " __VA_ARGS__)
#define LOGI(...) fprintf(stderr, "INFO: " __VA_ARGS__)

	/// @brief Helper macro to test the result of Vulkan calls which can return an
	/// error.
#define VK_CHECK(x)                                                                     \
	do                                                                                  \
	{                                                                                   \
		VkResult err = x;                                                               \
		if (err)                                                                        \
		{                                                                               \
			LOGE("Detected Vulkan error %d at %s:%d.\n", int(err), __FILE__, __LINE__); \
			abort();                                                                    \
		}                                                                               \
	} while (0)

#define ASSERT_VK_HANDLE(handle)                                    \
	do                                                              \
	{                                                               \
		if ((handle) == VK_NULL_HANDLE)                             \
		{                                                           \
			LOGE("Handle is NULL at %s:%d.\n", __FILE__, __LINE__); \
			abort();                                                \
		}                                                           \
	} while (0)


	class VulkanHelpers
	{
	public:
		static VkCommandBuffer BeginSingleTimeCommands();
		static void EndSingleTimeCommands(VkCommandBuffer commandBuffer);

		static uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
		static void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
			VkBuffer& buffer, VkDeviceMemory& bufferMemory);
		static void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

		static bool CheckDeviceExtensionSupport(VkPhysicalDevice physicalDevice);
		static SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
	};
}
