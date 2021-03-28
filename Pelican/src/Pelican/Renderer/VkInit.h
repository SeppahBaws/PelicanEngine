#pragma once

#include <vulkan/vulkan.hpp>

namespace Pelican
{
	namespace VkInit
	{
		//-----------------------------------------------------------
		// Instance related infos
		//-----------------------------------------------------------

		vk::ApplicationInfo ApplicationInfo();
		vk::InstanceCreateInfo InstanceCreateInfo(const vk::ApplicationInfo* appInfo, const std::vector<const char*>& extensions);
		vk::DebugUtilsMessengerCreateInfoEXT DebugUtilsMessengerCreateInfo(PFN_vkDebugUtilsMessengerCallbackEXT callback);


		//-----------------------------------------------------------
		// Device related infos
		//-----------------------------------------------------------

		vk::DeviceQueueCreateInfo DeviceQueueCreateInfo(uint32_t queueFamilyIndex, uint32_t queueCount, const float* pQueuePriorities);
		vk::DeviceCreateInfo DeviceCreateInfo(const std::vector<vk::DeviceQueueCreateInfo>& queueCreateInfos, const vk::PhysicalDeviceFeatures* enabledFeatures, const std::vector<const char*>& enabledExtensionNames);


		//-----------------------------------------------------------
		// Swap chain related infos
		//-----------------------------------------------------------

		vk::SwapchainCreateInfoKHR SwapchainCreateInfo();


		//-----------------------------------------------------------
		// Graphics pipeline and render pass related infos
		//-----------------------------------------------------------

		vk::PipelineVertexInputStateCreateInfo VertexInputStateCreateInfo();
		vk::PipelineInputAssemblyStateCreateInfo InputAssemblyStateCreateInfo(vk::PrimitiveTopology topology);
		vk::PipelineRasterizationStateCreateInfo RasterizationStateCreateInfo(vk::PolygonMode polygonMode, vk::CullModeFlags cullMode);
		vk::PipelineMultisampleStateCreateInfo MultisampleStateCreateInfo();
		vk::PipelineDepthStencilStateCreateInfo DepthStencilCreateInfo(bool depthTest, bool depthWrite, vk::CompareOp compareOp);
		vk::PipelineColorBlendAttachmentState ColorBlendAttachmentState();
		vk::PipelineColorBlendStateCreateInfo ColorBlendState();
		vk::PipelineLayoutCreateInfo PipelineLayoutCreateInfo();

		vk::RenderPassBeginInfo RenderPassBeginInfo();


		//-----------------------------------------------------------
		// Image related infos
		//-----------------------------------------------------------

		vk::ImageCreateInfo ImageCreateInfo(vk::ImageType imageType);
		vk::ImageViewCreateInfo ImageViewCreateInfo(vk::Image image, vk::ImageViewType viewType, vk::Format format, vk::ImageViewCreateFlags flags = {});
		vk::ImageMemoryBarrier ImageMemoryBarrier(vk::Image image, vk::ImageLayout oldLayout = {}, vk::ImageLayout newLayout = {});


		//-----------------------------------------------------------
		// Uncategorized infos
		//-----------------------------------------------------------

		vk::RenderPassCreateInfo RenderPassCreateInfo(vk::RenderPassCreateFlags flags = {});
		vk::DescriptorSetLayoutCreateInfo DescriptorSetLayoutCreateInfo(vk::DescriptorSetLayoutCreateFlags flags = {});

		vk::CommandPoolCreateInfo CommandPoolCreateInfo(uint32_t queueFamilyIndex, vk::CommandPoolCreateFlags flags = {});
		vk::CommandBufferAllocateInfo CommandBufferAllocateInfo(vk::CommandPool pool, uint32_t count = 1, vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary);
		vk::CommandBufferBeginInfo CommandBufferBeginInfo(vk::CommandBufferUsageFlags flags = {});

		vk::SemaphoreCreateInfo SemaphoreCreateInfo();
		vk::FenceCreateInfo FenceCreateInfo(vk::FenceCreateFlags flags = {});
	}
}
