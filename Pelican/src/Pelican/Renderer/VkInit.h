#pragma once

#include <vulkan/vulkan.h>

namespace Pelican
{
	namespace VkInit
	{
		//-----------------------------------------------------------
		// Instance related infos
		//-----------------------------------------------------------

		VkApplicationInfo ApplicationInfo();
		VkInstanceCreateInfo InstanceCreateInfo(const VkApplicationInfo* appInfo, const std::vector<const char*>& extensions);
		VkDebugUtilsMessengerCreateInfoEXT DebugUtilsMessengerCreateInfo(PFN_vkDebugUtilsMessengerCallbackEXT callback);


		//-----------------------------------------------------------
		// Device related infos
		//-----------------------------------------------------------

		VkDeviceQueueCreateInfo DeviceQueueCreateInfo(uint32_t queueFamilyIndex, uint32_t queueCount, const float* pQueuePriorities);
		VkDeviceCreateInfo DeviceCreateInfo(const std::vector<VkDeviceQueueCreateInfo>& queueCreateInfos, const VkPhysicalDeviceFeatures* enabledFeatures, const std::vector<const char*>& enabledExtensionNames);


		//-----------------------------------------------------------
		// Swap chain related infos
		//-----------------------------------------------------------

		VkSwapchainCreateInfoKHR SwapchainCreateInfo();


		//-----------------------------------------------------------
		// Graphics pipeline and render pass related infos
		//-----------------------------------------------------------

		VkPipelineVertexInputStateCreateInfo VertexInputStateCreateInfo();
		VkPipelineInputAssemblyStateCreateInfo InputAssemblyStateCreateInfo(VkPrimitiveTopology topology);
		VkPipelineRasterizationStateCreateInfo RasterizationStateCreateInfo(VkPolygonMode polygonMode, VkCullModeFlags cullMode);
		VkPipelineMultisampleStateCreateInfo MultisampleStateCreateInfo();
		VkPipelineDepthStencilStateCreateInfo DepthStencilCreateInfo(bool depthTest, bool depthWrite, VkCompareOp compareOp);
		VkPipelineColorBlendAttachmentState ColorBlendAttachmentState();
		VkPipelineColorBlendStateCreateInfo ColorBlendState();
		VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo();

		VkRenderPassBeginInfo RenderPassBeginInfo();


		//-----------------------------------------------------------
		// Image related infos
		//-----------------------------------------------------------

		VkImageCreateInfo ImageCreateInfo(VkImageType imageType);
		VkImageViewCreateInfo ImageViewCreateInfo(VkImage image, VkImageViewType viewType, VkFormat format, VkImageViewCreateFlags flags = 0);
		VkImageMemoryBarrier ImageMemoryBarrier();


		//-----------------------------------------------------------
		// Uncategorized infos
		//-----------------------------------------------------------

		VkRenderPassCreateInfo RenderPassCreateInfo(VkRenderPassCreateFlags flags = 0);
		VkDescriptorSetLayoutCreateInfo DescriptorSetLayoutCreateInfo(VkDescriptorSetLayoutCreateFlags flags = 0);

		VkCommandPoolCreateInfo CommandPoolCreateInfo(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags = 0);
		VkCommandBufferAllocateInfo CommandBufferAllocateInfo(VkCommandPool pool, uint32_t count = 1, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
		VkCommandBufferBeginInfo CommandBufferBeginInfo(VkCommandBufferUsageFlags flags = 0);

		VkSemaphoreCreateInfo SemaphoreCreateInfo();
		VkFenceCreateInfo FenceCreateInfo(VkFenceCreateFlags flags = 0);
	}
}
