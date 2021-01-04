#include "PelicanPCH.h"
#include "ImGuiWrapper.h"

#include "Pelican/Application.h"
#include "Pelican/Time.h"

#include "Pelican/Input/Input.h"

#include "Pelican/Renderer/VulkanDevice.h"
#include "Pelican/Renderer/VulkanHelpers.h"
#include "Pelican/Renderer/VulkanRenderer.h"
#include "Pelican/Renderer/VulkanShader.h"

#include <imgui.h>
// ReSharper disable file CppUnusedIncludeDirective
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

namespace Pelican
{
	ImGuiWrapper::ImGuiWrapper(VulkanDevice* pDevice)
		: m_pDevice(pDevice)
	{
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		// io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

		m_pShader = new VulkanShader("res/shaders/imgui/imgui.vert.spv", "res/shaders/imgui/imgui.frag.spv");
	}

	ImGuiWrapper::~ImGuiWrapper()
	{
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

		delete m_pShader;

		// TODO: destroy vertex and index buffers
		vkDestroyBuffer(m_pDevice->GetDevice(), m_IndexBuffer, nullptr);
		vkFreeMemory(m_pDevice->GetDevice(), m_IndexBufferMemory, nullptr);
		vkDestroyBuffer(m_pDevice->GetDevice(), m_VertexBuffer, nullptr);
		vkFreeMemory(m_pDevice->GetDevice(), m_VertexBufferMemory, nullptr);
		vkDestroyImage(m_pDevice->GetDevice(), m_FontImage, nullptr);
		vkDestroyImageView(m_pDevice->GetDevice(), m_FontImageView, nullptr);
		vkFreeMemory(m_pDevice->GetDevice(), m_FontMemory, nullptr);
		vkDestroySampler(m_pDevice->GetDevice(), m_Sampler, nullptr);
		vkDestroyPipelineCache(m_pDevice->GetDevice(), m_PipelineCache, nullptr);
		vkDestroyPipeline(m_pDevice->GetDevice(), m_Pipeline, nullptr);
		vkDestroyPipelineLayout(m_pDevice->GetDevice(), m_PipelineLayout, nullptr);
		vkDestroyDescriptorPool(m_pDevice->GetDevice(), m_DescriptorPool, nullptr);
		vkDestroyDescriptorSetLayout(m_pDevice->GetDevice(), m_DescriptorSetLayout, nullptr);
	}

	void ImGuiWrapper::Init(float width, float height)
	{
		ImGui::StyleColorsDark();
		ImGuiStyle& style = ImGui::GetStyle();
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;

		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2(width, height);
		io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
	}

	// vks::tools excerpt
	void setImageLayout(
		VkCommandBuffer cmdbuffer,
		VkImage image,
		VkImageLayout oldImageLayout,
		VkImageLayout newImageLayout,
		VkImageSubresourceRange subresourceRange,
		VkPipelineStageFlags srcStageMask,
		VkPipelineStageFlags dstStageMask)
	{
		// Create an image barrier object
		VkImageMemoryBarrier imageMemoryBarrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
		imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.oldLayout = oldImageLayout;
		imageMemoryBarrier.newLayout = newImageLayout;
		imageMemoryBarrier.image = image;
		imageMemoryBarrier.subresourceRange = subresourceRange;

		// Source layouts (old)
		// Source access mask controls actions that have to be finished on the old layout
		// before it will be transitioned to the new layout
		switch (oldImageLayout)
		{
		case VK_IMAGE_LAYOUT_UNDEFINED:
			// Image layout is undefined (or does not matter)
			// Only valid as initial layout
			// No flags required, listed only for completeness
			imageMemoryBarrier.srcAccessMask = 0;
			break;

		case VK_IMAGE_LAYOUT_PREINITIALIZED:
			// Image is preinitialized
			// Only valid as initial layout for linear images, preserves memory contents
			// Make sure host writes have been finished
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			// Image is a color attachment
			// Make sure any writes to the color buffer have been finished
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			// Image is a depth/stencil attachment
			// Make sure any writes to the depth/stencil buffer have been finished
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			// Image is a transfer source
			// Make sure any reads from the image have been finished
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			break;

		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			// Image is a transfer destination
			// Make sure any writes to the image have been finished
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			// Image is read by a shader
			// Make sure any shader reads from the image have been finished
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
			break;
		default:
			// Other source layouts aren't handled (yet)
			break;
		}

		// Target layouts (new)
		// Destination access mask controls the dependency for the new image layout
		switch (newImageLayout)
		{
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			// Image will be used as a transfer destination
			// Make sure any writes to the image have been finished
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			// Image will be used as a transfer source
			// Make sure any reads from the image have been finished
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			break;

		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			// Image will be used as a color attachment
			// Make sure any writes to the color buffer have been finished
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			// Image layout will be used as a depth/stencil attachment
			// Make sure any writes to depth/stencil buffer have been finished
			imageMemoryBarrier.dstAccessMask = imageMemoryBarrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			// Image will be read in a shader (sampler, input attachment)
			// Make sure any writes to the image have been finished
			if (imageMemoryBarrier.srcAccessMask == 0)
			{
				imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
			}
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			break;
		default:
			// Other source layouts aren't handled (yet)
			break;
		}

		// Put barrier inside setup command buffer
		vkCmdPipelineBarrier(
			cmdbuffer,
			srcStageMask,
			dstStageMask,
			0,
			0, nullptr,
			0, nullptr,
			1, &imageMemoryBarrier);
	}

	// Fixed sub resource on first mip level and layer
	void setImageLayout(
		VkCommandBuffer cmdbuffer,
		VkImage image,
		VkImageAspectFlags aspectMask,
		VkImageLayout oldImageLayout,
		VkImageLayout newImageLayout,
		VkPipelineStageFlags srcStageMask,
		VkPipelineStageFlags dstStageMask)
	{
		VkImageSubresourceRange subresourceRange = {};
		subresourceRange.aspectMask = aspectMask;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = 1;
		subresourceRange.layerCount = 1;
		setImageLayout(cmdbuffer, image, oldImageLayout, newImageLayout, subresourceRange, srcStageMask, dstStageMask);
	}

	// TODO: find usage for these unreferenced parameters
	void ImGuiWrapper::InitResources(VkRenderPass renderPass, VkQueue /*copyQueue*/, const std::string& /*shadersPath*/)
	{
		ImGuiIO& io = ImGui::GetIO();

		// Create font texture
		unsigned char* fontData;
		int texWidth, texHeight;
		io.Fonts->GetTexDataAsRGBA32(&fontData, &texWidth, &texHeight);
		VkDeviceSize uploadSize = texWidth * texHeight * 4 * sizeof(char);

		// Create target image for copy
		VkImageCreateInfo imageInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
		imageInfo.extent.width = texWidth;
		imageInfo.extent.height = texHeight;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		if (vkCreateImage(m_pDevice->GetDevice(), &imageInfo, nullptr, &m_FontImage) != VK_SUCCESS)
		{
			ASSERT_MSG(false, "Failed to create ImGui font image!");
		}

		VkMemoryRequirements memReqs;
		vkGetImageMemoryRequirements(m_pDevice->GetDevice(), m_FontImage, &memReqs);
		VkMemoryAllocateInfo memAllocInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
		memAllocInfo.allocationSize = memReqs.size;
		memAllocInfo.memoryTypeIndex = VulkanHelpers::FindMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		if (vkAllocateMemory(m_pDevice->GetDevice(), &memAllocInfo, nullptr, &m_FontMemory) != VK_SUCCESS)
		{
			ASSERT_MSG(false, "Failed to allocate ImGui font image memory!");
		}
		vkBindImageMemory(m_pDevice->GetDevice(), m_FontImage, m_FontMemory, 0);

		// Image view
		VkImageViewCreateInfo viewInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
		viewInfo.image = m_FontImage;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(m_pDevice->GetDevice(), &viewInfo, nullptr, &m_FontImageView) != VK_SUCCESS)
		{
			ASSERT_MSG(false, "Failed to create ImGui font image view!");
		}

		// Staging buffer for font data upload
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		VulkanHelpers::CreateBuffer(uploadSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* stagingBufferData;
		vkMapMemory(m_pDevice->GetDevice(), stagingBufferMemory, 0, uploadSize, 0, &stagingBufferData);
		memcpy(stagingBufferData, fontData, uploadSize);
		vkUnmapMemory(m_pDevice->GetDevice(), stagingBufferMemory);

		// Copy buffer data to font image
		VkCommandBuffer copyCmd = VulkanHelpers::BeginSingleTimeCommands();

		// Prepare for transfer
		setImageLayout(
			copyCmd,
			m_FontImage,
			VK_IMAGE_ASPECT_COLOR_BIT,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_PIPELINE_STAGE_HOST_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT);

		// VkImageSubresourceRange subresourceRange = {};
		// subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		// subresourceRange.baseMipLevel = 0;
		// subresourceRange.levelCount = 1;
		// subresourceRange.layerCount = 1;
		//
		// // Create an image barrier object
		// VkImageMemoryBarrier imageMemoryBarrier = {};
		// imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		// imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		// imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		// imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		// imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		// imageMemoryBarrier.image = m_FontImage;
		// imageMemoryBarrier.subresourceRange = subresourceRange;
		// imageMemoryBarrier.srcAccessMask = 0;
		// imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		//
		// vkCmdPipelineBarrier(
		// 	copyCmd,
		// 	VK_PIPELINE_STAGE_HOST_BIT,
		// 	VK_PIPELINE_STAGE_TRANSFER_BIT,
		// 	0,
		// 	0, nullptr,
		// 	0, nullptr,
		// 	1, &imageMemoryBarrier);

		// Copy
		VkBufferImageCopy bufferCopyRegion = {};
		bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		bufferCopyRegion.imageSubresource.layerCount = 1;
		bufferCopyRegion.imageExtent.width = texWidth;
		bufferCopyRegion.imageExtent.height = texHeight;
		bufferCopyRegion.imageExtent.depth = 1;

		vkCmdCopyBufferToImage(
			copyCmd,
			stagingBuffer,
			m_FontImage,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&bufferCopyRegion
		);

		// Prepare for shader read
		setImageLayout(
			copyCmd,
			m_FontImage,
			VK_IMAGE_ASPECT_COLOR_BIT,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
		// subresourceRange = {};
		// subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		// subresourceRange.baseMipLevel = 0;
		// subresourceRange.levelCount = 1;
		// subresourceRange.layerCount = 1;
		//
		// // Create an image barrier object
		// imageMemoryBarrier = {};
		// imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		// imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		// imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		// imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		// imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		// imageMemoryBarrier.image = m_FontImage;
		// imageMemoryBarrier.subresourceRange = subresourceRange;
		// // imageMemoryBarrier.srcAccessMask = 0;
		// // imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		// imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
		// imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		//
		// vkCmdPipelineBarrier(
		// 	copyCmd,
		// 	VK_PIPELINE_STAGE_TRANSFER_BIT,
		// 	VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		// 	0,
		// 	0, nullptr,
		// 	0, nullptr,
		// 	1, &imageMemoryBarrier);

		VulkanHelpers::EndSingleTimeCommands(copyCmd);

		// Free the staging buffer resources
		vkDestroyBuffer(m_pDevice->GetDevice(), stagingBuffer, nullptr);
		vkFreeMemory(m_pDevice->GetDevice(), stagingBufferMemory, nullptr);

		// Font texture sampler
		VkSamplerCreateInfo samplerInfo = { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
		samplerInfo.maxAnisotropy = 1.0f;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeV = samplerInfo.addressModeU;
		samplerInfo.addressModeW = samplerInfo.addressModeU;
		samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

		if (vkCreateSampler(m_pDevice->GetDevice(), &samplerInfo, nullptr, &m_Sampler) != VK_SUCCESS)
		{
			ASSERT_MSG(false, "Failed to create ImGui font texture sampler!");
		}

		// Descriptor pool
		std::vector<VkDescriptorPoolSize> poolSizes =
		{
			VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 }
		};

		VkDescriptorPoolCreateInfo descriptorPoolInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
		descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		descriptorPoolInfo.pPoolSizes = poolSizes.data();
		descriptorPoolInfo.maxSets = 2;

		if (vkCreateDescriptorPool(m_pDevice->GetDevice(), &descriptorPoolInfo, nullptr, &m_DescriptorPool) != VK_SUCCESS)
		{
			ASSERT_MSG(false, "Failed to create ImGui descriptor pool!");
		}

		// Descriptor set layout
		std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings =
		{
			VkDescriptorSetLayoutBinding{ 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT }
		};

		VkDescriptorSetLayoutCreateInfo descriptorLayout = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
		descriptorLayout.pBindings = setLayoutBindings.data();
		descriptorLayout.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());

		if (vkCreateDescriptorSetLayout(m_pDevice->GetDevice(), &descriptorLayout, nullptr, &m_DescriptorSetLayout) != VK_SUCCESS)
		{
			ASSERT_MSG(false, "Failed to create ImGui descriptor set layout!");
		}

		// Descriptor set
		VkDescriptorSetAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
		allocInfo.descriptorPool = m_DescriptorPool;
		allocInfo.pSetLayouts = &m_DescriptorSetLayout;
		allocInfo.descriptorSetCount = 1;

		if (vkAllocateDescriptorSets(m_pDevice->GetDevice(), &allocInfo, &m_DescriptorSet) != VK_SUCCESS)
		{
			ASSERT_MSG(false, "Failed to allocate ImGui descriptor sets!");
		}

		VkDescriptorImageInfo fontDescriptor = {};
		fontDescriptor.sampler = m_Sampler;
		fontDescriptor.imageView = m_FontImageView;
		fontDescriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkWriteDescriptorSet writeDescriptorSet = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		writeDescriptorSet.dstSet = m_DescriptorSet;
		writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeDescriptorSet.dstBinding = 0;
		writeDescriptorSet.pImageInfo = &fontDescriptor;
		writeDescriptorSet.descriptorCount = 1;

		std::vector<VkWriteDescriptorSet> writeDescriptorSets =
		{
			writeDescriptorSet
		};
		vkUpdateDescriptorSets(m_pDevice->GetDevice(), static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);

		// Pipeline cache
		VkPipelineCacheCreateInfo pipelineCacheCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO };
		if (vkCreatePipelineCache(m_pDevice->GetDevice(), &pipelineCacheCreateInfo, nullptr, &m_PipelineCache) != VK_SUCCESS)
		{
			ASSERT_MSG(false, "Failed to create ImGui pipeline cache!");
		}

		// Pipeline layout
		// Push constants for UI rendering parameters
		VkPushConstantRange pushConstantRange = {};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(PushConstBlock);

		VkPipelineLayoutCreateInfo pipelineLayoutCI = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
		pipelineLayoutCI.setLayoutCount = 1;
		pipelineLayoutCI.pSetLayouts = &m_DescriptorSetLayout;
		pipelineLayoutCI.pushConstantRangeCount = 1;
		pipelineLayoutCI.pPushConstantRanges = &pushConstantRange;

		if (vkCreatePipelineLayout(m_pDevice->GetDevice(), &pipelineLayoutCI, nullptr, &m_PipelineLayout) != VK_SUCCESS)
		{
			ASSERT_MSG(false, "Failed to create ImGui pipeline layout!");
		}

		// Setup graphics pipeline for UI rendering
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = { VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
		inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssemblyState.flags = 0;
		inputAssemblyState.primitiveRestartEnable = VK_FALSE;

		VkPipelineRasterizationStateCreateInfo rasterizationState = { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
		rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizationState.cullMode = VK_CULL_MODE_NONE;
		rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizationState.flags = 0;
		rasterizationState.depthClampEnable = VK_FALSE;
		rasterizationState.lineWidth = 1.0f;

		// Enable blending
		VkPipelineColorBlendAttachmentState blendAttachmentState = {};
		blendAttachmentState.blendEnable = VK_TRUE;
		blendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		blendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		blendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		blendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
		blendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		blendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		blendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

		VkPipelineColorBlendStateCreateInfo colorBlendState = { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
		colorBlendState.attachmentCount = 1;
		colorBlendState.pAttachments = &blendAttachmentState;

		VkPipelineDepthStencilStateCreateInfo depthStencilState = { VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
		depthStencilState.depthTestEnable = VK_FALSE;
		depthStencilState.depthWriteEnable = VK_FALSE;
		depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		depthStencilState.back.compareOp = VK_COMPARE_OP_ALWAYS;

		VkPipelineViewportStateCreateInfo viewportState = { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;
		viewportState.flags = 0;

		VkPipelineMultisampleStateCreateInfo multisampleState = { VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
		multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampleState.flags = 0;

		std::vector<VkDynamicState> dynamicStateEnables =
		{
			VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR
		};
		VkPipelineDynamicStateCreateInfo dynamicState = { VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
		dynamicState.pDynamicStates = dynamicStateEnables.data();
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());
		dynamicState.flags = 0;

		std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages{};

		VkGraphicsPipelineCreateInfo pipelineCreateInfo = { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
		pipelineCreateInfo.layout = m_PipelineLayout;
		pipelineCreateInfo.renderPass = renderPass;
		pipelineCreateInfo.flags = 0;
		pipelineCreateInfo.basePipelineIndex = -1;
		pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;

		pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
		pipelineCreateInfo.pRasterizationState = &rasterizationState;
		pipelineCreateInfo.pColorBlendState = &colorBlendState;
		pipelineCreateInfo.pMultisampleState = &multisampleState;
		pipelineCreateInfo.pViewportState = &viewportState;
		pipelineCreateInfo.pDepthStencilState = &depthStencilState;
		pipelineCreateInfo.pDynamicState = &dynamicState;
		pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
		pipelineCreateInfo.pStages = shaderStages.data();

		// Vertex bindings and attributes based on ImGui vertex definition
		std::vector<VkVertexInputBindingDescription> vertexInputBindings =
		{
			VkVertexInputBindingDescription{ 0, sizeof(ImDrawVert), VK_VERTEX_INPUT_RATE_VERTEX }
		};
		std::vector<VkVertexInputAttributeDescription> vertexInputAttributes =
		{
			VkVertexInputAttributeDescription{ 0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(ImDrawVert, pos) },
			VkVertexInputAttributeDescription{ 1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(ImDrawVert, uv) },
			VkVertexInputAttributeDescription{ 2, 0, VK_FORMAT_R8G8B8_UNORM, offsetof(ImDrawVert, col) }
		};

		VkPipelineVertexInputStateCreateInfo vertexInputState = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
		vertexInputState.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexInputBindings.size());
		vertexInputState.pVertexBindingDescriptions = vertexInputBindings.data();
		vertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputAttributes.size());
		vertexInputState.pVertexAttributeDescriptions = vertexInputAttributes.data();

		pipelineCreateInfo.pVertexInputState = &vertexInputState;

		shaderStages[0] = m_pShader->GetShaderStages()[0];
		shaderStages[1] = m_pShader->GetShaderStages()[1];

		if (vkCreateGraphicsPipelines(m_pDevice->GetDevice(), m_PipelineCache, 1, &pipelineCreateInfo, nullptr, &m_Pipeline) != VK_SUCCESS)
		{
			ASSERT_MSG(false, "Failed to create ImGui graphics pipeline!");
		}
	}

	void ImGuiWrapper::UpdateIO()
	{
		ImGuiIO& io = ImGui::GetIO();

		const Window::Params windowParams = Application::Get().GetWindow()->GetParams();
		io.DisplaySize = ImVec2(static_cast<float>(windowParams.width), static_cast<float>(windowParams.height));
		io.DeltaTime = Time::GetDeltaTime();

		io.MousePos = ImVec2(Input::GetMousePos().x, Input::GetMousePos().y);
		io.MouseDown[0] = Input::GetMouseButton(MouseCode::ButtonLeft);
		io.MouseDown[1] = Input::GetMouseButton(MouseCode::ButtonRight);
		io.MouseWheel = Input::GetScroll();
		io.KeyCtrl = Input::GetKey(KeyCode::LeftControl) || Input::GetKey(KeyCode::RightControl);
		io.KeyShift = Input::GetKey(KeyCode::LeftShift) || Input::GetKey(KeyCode::RightShift);
		io.KeyAlt = Input::GetKey(KeyCode::LeftAlt) || Input::GetKey(KeyCode::RightAlt);
		io.KeySuper = Input::GetKey(KeyCode::LeftSuper) || Input::GetKey(KeyCode::RightSuper);
	}

	void ImGuiWrapper::NewFrame()
	{
		ImGui::NewFrame();
	}

	void ImGuiWrapper::Render()
	{
		ImGuiIO& io = ImGui::GetIO();

		ImGui::Render();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
	}

	void ImGuiWrapper::UpdateBuffers()
	{
		ImDrawData* imDrawData = ImGui::GetDrawData();

		// Note: alignment is done inside buffer creation
		const VkDeviceSize vertexBufferSize = imDrawData->TotalVtxCount * sizeof(ImDrawVert);
		const VkDeviceSize indexBufferSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);

		if (vertexBufferSize == 0 || indexBufferSize == 0)
		{
			return;
		}

		// Update buffers only if vertex or index count has been changed compared to current buffer size

		// Vertex buffer
		if (m_VertexBuffer == VK_NULL_HANDLE || m_VertexCount != imDrawData->TotalVtxCount)
		{
			if (m_VBMapped)
			{
				vkUnmapMemory(m_pDevice->GetDevice(), m_VertexBufferMemory);
				m_VBMapped = nullptr;
			}

			if (m_VertexBuffer)
				vkDestroyBuffer(m_pDevice->GetDevice(), m_VertexBuffer, nullptr);
			if (m_VertexBufferMemory)
				vkFreeMemory(m_pDevice->GetDevice(), m_VertexBufferMemory, nullptr);

			VulkanHelpers::CreateBuffer(vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, m_VertexBuffer, m_VertexBufferMemory);
			m_VertexCount = imDrawData->TotalVtxCount;
			vkMapMemory(m_pDevice->GetDevice(), m_VertexBufferMemory, 0, VK_WHOLE_SIZE, 0, &m_VBMapped);
		}

		// Index buffer
		if ((m_IndexBuffer == VK_NULL_HANDLE) || (m_IndexCount < imDrawData->TotalIdxCount))
		{
			if (m_IBMapped)
			{
				vkUnmapMemory(m_pDevice->GetDevice(), m_IndexBufferMemory);
				m_IBMapped = nullptr;
			}

			if (m_IndexBuffer)
				vkDestroyBuffer(m_pDevice->GetDevice(), m_IndexBuffer, nullptr);
			if (m_IndexBufferMemory)
				vkFreeMemory(m_pDevice->GetDevice(), m_IndexBufferMemory, nullptr);

			VulkanHelpers::CreateBuffer(indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, m_IndexBuffer, m_IndexBufferMemory);
			m_IndexCount = imDrawData->TotalIdxCount;
			vkMapMemory(m_pDevice->GetDevice(), m_IndexBufferMemory, 0, VK_WHOLE_SIZE, 0, &m_IBMapped);
		}

		// Upload data
		ImDrawVert* vtxDst = (ImDrawVert*)m_VBMapped;
		ImDrawIdx* idxDst = (ImDrawIdx*)m_IBMapped;

		for (int n = 0; n < imDrawData->CmdListsCount; n++)
		{
			const ImDrawList* cmdList = imDrawData->CmdLists[n];
			memcpy(vtxDst, cmdList->VtxBuffer.Data, cmdList->VtxBuffer.Size * sizeof(ImDrawVert));
			memcpy(idxDst, cmdList->IdxBuffer.Data, cmdList->IdxBuffer.Size * sizeof(ImDrawIdx));
			vtxDst += cmdList->VtxBuffer.Size;
			idxDst += cmdList->IdxBuffer.Size;
		}

		// Flush to make writes visible to GPU
		{
			VkMappedMemoryRange mappedRange = {};
			mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
			mappedRange.memory = m_VertexBufferMemory;
			mappedRange.offset = 0;
			mappedRange.size = VK_WHOLE_SIZE;
			if (vkFlushMappedMemoryRanges(m_pDevice->GetDevice(), 1, &mappedRange) != VK_SUCCESS)
			{
				ASSERT_MSG(false, "Failed to flush vertex buffer mapped range!");
			}
		}
		{
			VkMappedMemoryRange mappedRange = {};
			mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
			mappedRange.memory = m_IndexBufferMemory;
			mappedRange.offset = 0;
			mappedRange.size = VK_WHOLE_SIZE;
			if (vkFlushMappedMemoryRanges(m_pDevice->GetDevice(), 1, &mappedRange) != VK_SUCCESS)
			{
				ASSERT_MSG(false, "Failed to flush index buffer mapped range!");
			}
		}
	}

	void ImGuiWrapper::DrawFrame()
	{
		ImGuiIO& io = ImGui::GetIO();

		VkCommandBuffer commandBuffer = VulkanRenderer::GetCurrentBuffer();

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0, 1, &m_DescriptorSet, 0, nullptr);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline);

		VkViewport viewport = {};
		viewport.width = io.DisplaySize.x;
		viewport.height = io.DisplaySize.y;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		// UI scale and translate via push constants
		m_pushConstBlock.scale = glm::vec2(2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y);
		m_pushConstBlock.translate = glm::vec2(-1.0f);
		vkCmdPushConstants(commandBuffer, m_PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstBlock), &m_pushConstBlock);

		// Render commands
		ImDrawData* imDrawData = ImGui::GetDrawData();
		int32_t vertexOffset = 0;
		int32_t indexOffset = 0;

		if (imDrawData->CmdListsCount > 0)
		{
			VkDeviceSize offsets[1] = { 0 };
			vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_VertexBuffer, offsets);
			vkCmdBindIndexBuffer(commandBuffer, m_IndexBuffer, 0, VK_INDEX_TYPE_UINT16);

			for (int32_t i = 0; i < imDrawData->CmdListsCount; i++)
			{
				const ImDrawList* cmdList = imDrawData->CmdLists[i];
				for (int32_t j = 0; j < cmdList->CmdBuffer.Size; j++)
				{
					const ImDrawCmd* pCmd = &cmdList->CmdBuffer[j];
					VkRect2D scissorRect;
					scissorRect.offset.x = std::max(static_cast<int32_t>(pCmd->ClipRect.x), 0);
					scissorRect.offset.y = std::max(static_cast<int32_t>(pCmd->ClipRect.y), 0);
					scissorRect.extent.width = static_cast<uint32_t>(pCmd->ClipRect.z - pCmd->ClipRect.x);
					scissorRect.extent.height = static_cast<uint32_t>(pCmd->ClipRect.w - pCmd->ClipRect.y);
					vkCmdSetScissor(commandBuffer, 0, 1, &scissorRect);
					vkCmdDrawIndexed(commandBuffer, pCmd->ElemCount, 1, indexOffset, vertexOffset, 0);
					indexOffset += pCmd->ElemCount;
				}
				vertexOffset += cmdList->VtxBuffer.Size;
			}
		}
	}
}
