#include "PelicanPCH.h"
#include "VkInit.h"

namespace Pelican
{
	vk::ApplicationInfo VkInit::ApplicationInfo()
	{
		vk::ApplicationInfo info{};

		return info;
	}

	vk::InstanceCreateInfo VkInit::InstanceCreateInfo(const vk::ApplicationInfo* appInfo,
		const std::vector<const char*>& extensions)
	{
		vk::InstanceCreateInfo info{};

		info.pApplicationInfo = appInfo;

		info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		info.ppEnabledExtensionNames = extensions.data();

		return info;
	}

	vk::DebugUtilsMessengerCreateInfoEXT VkInit::DebugUtilsMessengerCreateInfo(PFN_vkDebugUtilsMessengerCallbackEXT callback)
	{
		vk::DebugUtilsMessengerCreateInfoEXT info{};

		using MsgSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT;
		using MsgType = vk::DebugUtilsMessageTypeFlagBitsEXT;

		info.messageSeverity = MsgSeverity::eVerbose | MsgSeverity::eWarning | MsgSeverity::eError;
		info.messageType = MsgType::eGeneral | MsgType::eValidation | MsgType::ePerformance;
		info.pfnUserCallback = callback;

		return info;
	}

	vk::DeviceQueueCreateInfo VkInit::DeviceQueueCreateInfo(uint32_t queueFamilyIndex, uint32_t queueCount,
		const float* pQueuePriorities)
	{
		vk::DeviceQueueCreateInfo info{};

		info.queueFamilyIndex = queueFamilyIndex;
		info.queueCount = queueCount;
		info.pQueuePriorities = pQueuePriorities;

		return info;
	}

	vk::DeviceCreateInfo VkInit::DeviceCreateInfo(const std::vector<vk::DeviceQueueCreateInfo>& queueCreateInfos,
		const vk::PhysicalDeviceFeatures* enabledFeatures, const std::vector<const char*>& enabledExtensionNames)
	{
		vk::DeviceCreateInfo info{};

		info.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		info.pQueueCreateInfos = queueCreateInfos.data();
		info.pEnabledFeatures = enabledFeatures;
		info.enabledExtensionCount = static_cast<uint32_t>(enabledExtensionNames.size());
		info.ppEnabledExtensionNames = enabledExtensionNames.data();

		return info;
	}

	vk::SwapchainCreateInfoKHR VkInit::SwapchainCreateInfo()
	{
		// Because the Swapchain create info struct is so large, we'll just return default values in here.
		vk::SwapchainCreateInfoKHR info{};

		return info;
	}

	vk::PipelineVertexInputStateCreateInfo VkInit::VertexInputStateCreateInfo()
	{
		vk::PipelineVertexInputStateCreateInfo info = {};

		return info;
	}

	vk::PipelineInputAssemblyStateCreateInfo VkInit::InputAssemblyStateCreateInfo(vk::PrimitiveTopology topology)
	{
		vk::PipelineInputAssemblyStateCreateInfo info = {};

		info.topology = topology;
		info.primitiveRestartEnable = false;

		return info;
	}

	vk::PipelineRasterizationStateCreateInfo VkInit::RasterizationStateCreateInfo(vk::PolygonMode polygonMode,
		vk::CullModeFlags cullMode)
	{
		vk::PipelineRasterizationStateCreateInfo info = {};

		info.depthClampEnable = false;
		info.rasterizerDiscardEnable = false;
		info.polygonMode = polygonMode;
		info.lineWidth = 1.0f;
		info.cullMode = cullMode;
		info.frontFace = vk::FrontFace::eCounterClockwise;
		info.depthBiasEnable = VK_FALSE;
		info.depthBiasConstantFactor = 0.0f;
		info.depthBiasClamp = 0.0f;
		info.depthBiasSlopeFactor = 0.0f;

		return info;
	}

	vk::PipelineMultisampleStateCreateInfo VkInit::MultisampleStateCreateInfo()
	{
		vk::PipelineMultisampleStateCreateInfo info = {};

		info.sampleShadingEnable = false;
		info.rasterizationSamples = vk::SampleCountFlagBits::e1;
		info.minSampleShading = 1.0f;
		info.pSampleMask = nullptr;
		info.alphaToCoverageEnable = false;
		info.alphaToOneEnable = false;

		return info;
	}

	vk::PipelineDepthStencilStateCreateInfo VkInit::DepthStencilCreateInfo(bool depthTest, bool depthWrite,
		vk::CompareOp compareOp)
	{
		vk::PipelineDepthStencilStateCreateInfo info{};

		info.depthTestEnable = depthTest ? true : false;
		info.depthWriteEnable = depthWrite ? true : false;
		info.depthCompareOp = depthTest ? compareOp : vk::CompareOp::eAlways;
		info.depthBoundsTestEnable = false;
		info.minDepthBounds = 0.0f;
		info.maxDepthBounds = 1.0f;
		info.stencilTestEnable = false;

		return info;
	}

	vk::PipelineColorBlendAttachmentState VkInit::ColorBlendAttachmentState()
	{
		vk::PipelineColorBlendAttachmentState colorBlendAttachment{};

		colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
		colorBlendAttachment.blendEnable = false;

		return colorBlendAttachment;
	}

	vk::PipelineColorBlendStateCreateInfo VkInit::ColorBlendState()
	{
		vk::PipelineColorBlendStateCreateInfo info{};

		info.logicOpEnable = false;
		info.logicOp = vk::LogicOp::eCopy;

		return info;
	}

	vk::PipelineLayoutCreateInfo VkInit::PipelineLayoutCreateInfo()
	{
		vk::PipelineLayoutCreateInfo info{};

		info.setLayoutCount = 0;
		info.pSetLayouts = nullptr;
		info.pushConstantRangeCount = 0;
		info.pPushConstantRanges = nullptr;

		return info;
	}

	vk::RenderPassBeginInfo VkInit::RenderPassBeginInfo()
	{
		vk::RenderPassBeginInfo info{};

		return info;
	}

	vk::ImageCreateInfo VkInit::ImageCreateInfo(vk::ImageType imageType)
	{
		vk::ImageCreateInfo info{};

		info.imageType = imageType;
		info.mipLevels = 1;
		info.arrayLayers = 1;
		info.initialLayout = vk::ImageLayout::eUndefined;
		info.sharingMode = vk::SharingMode::eExclusive;
		info.samples = vk::SampleCountFlagBits::e1;

		return info;
	}

	vk::ImageViewCreateInfo VkInit::ImageViewCreateInfo(vk::Image image, vk::ImageViewType viewType, vk::Format format,
		vk::ImageViewCreateFlags flags)
	{
		vk::ImageViewCreateInfo info{};
		info.flags = flags;

		info.image = image;
		info.viewType = viewType;
		info.format = format;

		return info;
	}

	vk::ImageMemoryBarrier VkInit::ImageMemoryBarrier(vk::Image image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout)
	{
		vk::ImageMemoryBarrier barrier{};

		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		return barrier;
	}

	vk::RenderPassCreateInfo VkInit::RenderPassCreateInfo(vk::RenderPassCreateFlags flags)
	{
		vk::RenderPassCreateInfo info{};
		info.flags = flags;

		return info;
	}

	vk::DescriptorSetLayoutCreateInfo VkInit::DescriptorSetLayoutCreateInfo(vk::DescriptorSetLayoutCreateFlags flags)
	{
		vk::DescriptorSetLayoutCreateInfo info{};
		info.flags = flags;

		return info;
	}

	vk::CommandPoolCreateInfo VkInit::CommandPoolCreateInfo(uint32_t queueFamilyIndex, vk::CommandPoolCreateFlags flags)
	{
		vk::CommandPoolCreateInfo info{};
		info.flags = flags;

		info.queueFamilyIndex = queueFamilyIndex;

		return info;
	}

	vk::CommandBufferAllocateInfo VkInit::CommandBufferAllocateInfo(vk::CommandPool pool, uint32_t count,
		vk::CommandBufferLevel level)
	{
		vk::CommandBufferAllocateInfo info{};

		info.commandPool = pool;
		info.commandBufferCount = count;
		info.level = level;

		return info;
	}

	vk::CommandBufferBeginInfo VkInit::CommandBufferBeginInfo(vk::CommandBufferUsageFlags flags)
	{
		vk::CommandBufferBeginInfo info{};
		info.flags = flags;

		info.pInheritanceInfo = nullptr;

		return info;
	}

	vk::SemaphoreCreateInfo VkInit::SemaphoreCreateInfo()
	{
		vk::SemaphoreCreateInfo info = {};

		return info;
	}

	vk::FenceCreateInfo VkInit::FenceCreateInfo(vk::FenceCreateFlags flags)
	{
		vk::FenceCreateInfo info = {};
		info.flags = flags;

		return info;
	}
}
