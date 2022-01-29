#pragma once

#include <vulkan/vulkan.hpp>

namespace Pelican
{
	class VulkanDevice;

	struct SwapChainSupportDetails
	{
		vk::SurfaceCapabilitiesKHR capabilities;
		std::vector<vk::SurfaceFormatKHR> formats;
		std::vector<vk::PresentModeKHR> presentModes;
	};

	inline std::vector<const char*> g_DeviceExtensions = {
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
		static vk::CommandBuffer BeginSingleTimeCommands();
		static void EndSingleTimeCommands(vk::CommandBuffer commandBuffer);

		static uint32_t FindMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);

		static bool CheckDeviceExtensionSupport(vk::PhysicalDevice physicalDevice);
		static SwapChainSupportDetails QuerySwapChainSupport(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface);
	};
}
