#include "PelicanPCH.h"
#include "VkInit.h"

namespace Pelican
{
	VkApplicationInfo VkInit::ApplicationInfo()
	{
		VkApplicationInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		info.pNext = nullptr;

		return info;
	}

	VkInstanceCreateInfo VkInit::InstanceCreateInfo(const VkApplicationInfo* appInfo,
		const std::vector<const char*>& extensions)
	{
		VkInstanceCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		info.pNext = nullptr;

		info.pApplicationInfo = appInfo;

		info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		info.ppEnabledExtensionNames = extensions.data();

		return info;
	}

	VkDebugUtilsMessengerCreateInfoEXT VkInit::DebugUtilsMessengerCreateInfo(PFN_vkDebugUtilsMessengerCallbackEXT callback)
	{
		VkDebugUtilsMessengerCreateInfoEXT info = {};
		info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		info.pNext = nullptr;

		info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		info.pfnUserCallback = callback;

		return info;
	}

	VkDeviceQueueCreateInfo VkInit::DeviceQueueCreateInfo(uint32_t queueFamilyIndex, uint32_t queueCount,
		const float* pQueuePriorities)
	{
		VkDeviceQueueCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		info.pNext = nullptr;

		info.queueFamilyIndex = queueFamilyIndex;
		info.queueCount = queueCount;
		info.pQueuePriorities = pQueuePriorities;

		return info;
	}

	VkDeviceCreateInfo VkInit::DeviceCreateInfo(const std::vector<VkDeviceQueueCreateInfo>& queueCreateInfos,
		const VkPhysicalDeviceFeatures* enabledFeatures, const std::vector<const char*>& enabledExtensionNames)
	{
		VkDeviceCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		info.pNext = nullptr;

		info.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		info.pQueueCreateInfos = queueCreateInfos.data();
		info.pEnabledFeatures = enabledFeatures;
		info.enabledExtensionCount = static_cast<uint32_t>(enabledExtensionNames.size());
		info.ppEnabledExtensionNames = enabledExtensionNames.data();

		return info;
	}

	VkSwapchainCreateInfoKHR VkInit::SwapchainCreateInfo()
	{
		// Because the Swapchain create info struct is so large, we'll just return default values in here.
		VkSwapchainCreateInfoKHR info = {};
		info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		info.pNext = nullptr;

		return info;
	}

	VkPipelineVertexInputStateCreateInfo VkInit::VertexInputStateCreateInfo()
	{
		VkPipelineVertexInputStateCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		info.pNext = nullptr;

		info.vertexBindingDescriptionCount = 0;
		info.pVertexBindingDescriptions = nullptr;
		info.vertexAttributeDescriptionCount = 0;
		info.pVertexAttributeDescriptions = nullptr;

		return info;
	}

	VkPipelineInputAssemblyStateCreateInfo VkInit::InputAssemblyStateCreateInfo(VkPrimitiveTopology topology)
	{
		VkPipelineInputAssemblyStateCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		info.pNext = nullptr;

		info.topology = topology;
		info.primitiveRestartEnable = VK_FALSE;

		return info;
	}

	VkPipelineRasterizationStateCreateInfo VkInit::RasterizationStateCreateInfo(VkPolygonMode polygonMode,
		VkCullModeFlags cullMode)
	{
		VkPipelineRasterizationStateCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		info.pNext = nullptr;

		info.depthClampEnable = VK_FALSE;
		info.rasterizerDiscardEnable = VK_FALSE;
		info.polygonMode = polygonMode;
		info.lineWidth = 1.0f;
		info.cullMode = cullMode;
		info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		info.depthBiasEnable = VK_FALSE;
		info.depthBiasConstantFactor = 0.0f;
		info.depthBiasClamp = 0.0f;
		info.depthBiasSlopeFactor = 0.0f;

		return info;
	}

	VkPipelineMultisampleStateCreateInfo VkInit::MultisampleStateCreateInfo()
	{
		VkPipelineMultisampleStateCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		info.pNext = nullptr;

		info.sampleShadingEnable = VK_FALSE;
		info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		info.minSampleShading = 1.0f;
		info.pSampleMask = nullptr;
		info.alphaToCoverageEnable = VK_FALSE;
		info.alphaToOneEnable = VK_FALSE;

		return info;
	}

	VkPipelineDepthStencilStateCreateInfo VkInit::DepthStencilCreateInfo(bool depthTest, bool depthWrite,
		VkCompareOp compareOp)
	{
		VkPipelineDepthStencilStateCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		info.pNext = nullptr;
		info.flags = 0;

		info.depthTestEnable = depthTest ? VK_TRUE : VK_FALSE;
		info.depthWriteEnable = depthWrite ? VK_TRUE : VK_FALSE;
		info.depthCompareOp = depthTest ? compareOp : VK_COMPARE_OP_ALWAYS;
		info.depthBoundsTestEnable = VK_FALSE;
		info.minDepthBounds = 0.0f;
		info.maxDepthBounds = 1.0f;
		info.stencilTestEnable = VK_FALSE;

		return info;
	}

	VkPipelineColorBlendAttachmentState VkInit::ColorBlendAttachmentState()
	{
		VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;

		return colorBlendAttachment;
	}

	VkPipelineColorBlendStateCreateInfo VkInit::ColorBlendState()
	{
		VkPipelineColorBlendStateCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		info.pNext = nullptr;

		info.logicOpEnable = VK_FALSE;
		info.logicOp = VK_LOGIC_OP_COPY;

		return info;
	}

	VkPipelineLayoutCreateInfo VkInit::PipelineLayoutCreateInfo()
	{
		VkPipelineLayoutCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		info.pNext = nullptr;
		info.flags = 0;

		info.setLayoutCount = 0;
		info.pSetLayouts = nullptr;
		info.pushConstantRangeCount = 0;
		info.pPushConstantRanges = nullptr;

		return info;
	}

	VkRenderPassBeginInfo VkInit::RenderPassBeginInfo()
	{
		VkRenderPassBeginInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		info.pNext = nullptr;

		return info;
	}

	VkImageCreateInfo VkInit::ImageCreateInfo(VkImageType imageType)
	{
		VkImageCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		info.pNext = nullptr;

		info.imageType = imageType;
		info.mipLevels = 1;
		info.arrayLayers = 1;
		info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		info.samples = VK_SAMPLE_COUNT_1_BIT;

		return info;
	}

	VkImageViewCreateInfo VkInit::ImageViewCreateInfo(VkImage image, VkImageViewType viewType, VkFormat format,
		VkImageViewCreateFlags flags)
	{
		VkImageViewCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		info.pNext = nullptr;
		info.flags = flags;

		info.image = image;
		info.viewType = viewType;
		info.format = format;

		return info;
	}

	VkImageMemoryBarrier VkInit::ImageMemoryBarrier()
	{
		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.pNext = nullptr;

		barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		barrier.newLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = nullptr;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		return barrier;
	}

	VkRenderPassCreateInfo VkInit::RenderPassCreateInfo(VkRenderPassCreateFlags flags)
	{
		VkRenderPassCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		info.pNext = nullptr;
		info.flags = flags;

		return info;
	}

	VkDescriptorSetLayoutCreateInfo VkInit::DescriptorSetLayoutCreateInfo(VkDescriptorSetLayoutCreateFlags flags)
	{
		VkDescriptorSetLayoutCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		info.pNext = nullptr;
		info.flags = flags;

		return info;
	}

	VkCommandPoolCreateInfo VkInit::CommandPoolCreateInfo(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags)
	{
		VkCommandPoolCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		info.pNext = nullptr;
		info.flags = flags;

		info.queueFamilyIndex = queueFamilyIndex;

		return info;
	}

	VkCommandBufferAllocateInfo VkInit::CommandBufferAllocateInfo(VkCommandPool pool, uint32_t count,
		VkCommandBufferLevel level)
	{
		VkCommandBufferAllocateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		info.pNext = nullptr;

		info.commandPool = pool;
		info.commandBufferCount = count;
		info.level = level;

		return info;
	}

	VkCommandBufferBeginInfo VkInit::CommandBufferBeginInfo(VkCommandBufferUsageFlags flags)
	{
		VkCommandBufferBeginInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		info.pNext = nullptr;
		info.flags = flags;

		info.pInheritanceInfo = nullptr;

		return info;
	}

	VkSemaphoreCreateInfo VkInit::SemaphoreCreateInfo()
	{
		VkSemaphoreCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		info.pNext = nullptr;
		info.flags = 0;

		return info;
	}

	VkFenceCreateInfo VkInit::FenceCreateInfo(VkFenceCreateFlags flags)
	{
		VkFenceCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		info.pNext = nullptr;
		info.flags = flags;

		return info;
	}
}
