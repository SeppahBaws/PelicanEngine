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

#pragma warning(push, 0)
	static void VkCheckResult(VkResult err)
	{
		if (err == 0)
			return;
		std::cerr << "[vulkan] Error: VkResult = " << err << std::endl;
		if (err < 0)
			abort();
	}
#pragma warning(pop)

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
