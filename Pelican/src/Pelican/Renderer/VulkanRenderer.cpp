#include "PelicanPCH.h"
#include "VulkanRenderer.h"
#include "VulkanHelpers.h"

#include <fstream>
#include <algorithm>
#include <cstdint>
#include <array>
#include <chrono>

#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Pelican/Application.h"

#include "VulkanProxy.h"
#include "UniformBufferObject.h"
#include "Camera.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "Vertex.h"
#include "VulkanTexture.h"
#include "VulkanShader.h"

namespace Pelican
{
	VulkanRenderer* VulkanRenderer::m_pInstance{};

	VulkanRenderer::VulkanRenderer()
	{
		ASSERT_MSG(!m_pInstance, "VulkanRenderer::m_pInstance is not nullptr, there can only be one instance at a time!");

		m_pInstance = this;
	}

	void VulkanRenderer::Initialize()
	{
		CreateInstance();
		SetupDebugMessenger();

		m_pDevice = new VulkanDevice(m_VkInstance);
		m_pSwapChain = new VulkanSwapChain(m_pDevice);

		CreateRenderPass();
		CreateDescriptorSetLayout();
		CreateGraphicsPipeline();
		CreateCommandPool();
		CreateDepthResources();
		CreateFramebuffers();

		m_pTexture = new VulkanTexture("res/textures/Brick/Brick_Color.jpg");

		CreateUniformBuffers();
		CreateDescriptorPool();
		CreateDescriptorSets();
		CreateCommandBuffers();
		CreateSyncObjects();
	}

	void VulkanRenderer::BeforeSceneCleanup()
	{
		m_pDevice->WaitIdle();

		CleanupSwapChain();
	}

	void VulkanRenderer::AfterSceneCleanup()
	{
		delete m_pTexture;
		m_pTexture = nullptr;

		vkDestroyDescriptorSetLayout(m_pDevice->GetDevice(), m_VkDescriptorSetLayout, nullptr);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			vkDestroySemaphore(m_pDevice->GetDevice(), m_VkRenderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(m_pDevice->GetDevice(), m_VkImageAvailableSemaphores[i], nullptr);
			vkDestroyFence(m_pDevice->GetDevice(), m_VkInFlightFences[i], nullptr);
		}

		vkDestroyCommandPool(m_pDevice->GetDevice(), m_VkCommandPool, nullptr);

		delete m_pSwapChain;
		m_pSwapChain = nullptr;

		delete m_pDevice;
		m_pDevice = nullptr;

		if (m_EnableValidationLayers)
		{
			VulkanProxy::DestroyDebugUtilsMessenger(m_VkInstance, m_VkDebugMessenger, nullptr);
		}

		vkDestroyInstance(m_VkInstance, nullptr);
	}

	void VulkanRenderer::BeginScene()
	{
		vkWaitForFences(m_pDevice->GetDevice(), 1, &m_VkInFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);

		VkResult result = vkAcquireNextImageKHR(m_pDevice->GetDevice(), m_pSwapChain->GetSwapChain(), UINT64_MAX, m_VkImageAvailableSemaphores[m_CurrentFrame], VK_NULL_HANDLE, &m_CurrentBuffer);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || m_FrameBufferResized)
		{
			m_FrameBufferResized = false;
			RecreateSwapChain();
			return;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			ASSERT_MSG(false, "failed to acquire swap chain image!");
		}

		if (m_VkImagesInFlight[m_CurrentBuffer] != VK_NULL_HANDLE)
		{
			vkWaitForFences(m_pDevice->GetDevice(), 1, &m_VkImagesInFlight[m_CurrentBuffer], VK_TRUE, UINT64_MAX);
		}
		m_VkImagesInFlight[m_CurrentBuffer] = m_VkInFlightFences[m_CurrentFrame];

		UpdateUniformBuffer(m_CurrentBuffer);


		BeginCommandBuffers();
	}

	void VulkanRenderer::EndScene()
	{
		EndCommandBuffers();


		VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };

		VkSemaphore waitSemaphores[] = { m_VkImageAvailableSemaphores[m_CurrentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_VkCommandBuffers[m_CurrentBuffer];

		VkSemaphore signalSemaphores[] = { m_VkRenderFinishedSemaphores[m_CurrentFrame] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		vkResetFences(m_pDevice->GetDevice(), 1, &m_VkInFlightFences[m_CurrentFrame]);

		if (vkQueueSubmit(m_pDevice->GetGraphicsQueue(), 1, &submitInfo, m_VkInFlightFences[m_CurrentFrame]) != VK_SUCCESS)
		{
			ASSERT_MSG(false, "failed to submit draw command buffer!");
		}

		VkPresentInfoKHR presentInfo = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { m_pSwapChain->GetSwapChain() };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &m_CurrentBuffer;
		presentInfo.pResults = nullptr;

		VkResult result = vkQueuePresentKHR(m_pDevice->GetPresentQueue(), &presentInfo);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
		{
			RecreateSwapChain();
		}
		else if (result != VK_SUCCESS)
		{
			ASSERT_MSG(false, "failed to present swap chain image!");
		}

		m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	void VulkanRenderer::SetCamera(Camera* pCamera)
	{
		m_pCamera = pCamera;
	}

	void VulkanRenderer::CreateInstance()
	{
		if (m_EnableValidationLayers && !CheckValidationLayerSupport())
		{
			ASSERT_MSG(false, "validation layers requested, but not available!");
		}

		VkApplicationInfo appInfo = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
		appInfo.pApplicationName = "Pelican Game";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "Pelican Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_2;

		VkInstanceCreateInfo createInfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
		createInfo.pApplicationInfo = &appInfo;

		// Extensions
		std::vector<const char*> extensions = GetRequiredExtensions();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		PrintExtensions();

		// Layers
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
		if (m_EnableValidationLayers)
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(m_ValidationLayers.size());
			createInfo.ppEnabledLayerNames = m_ValidationLayers.data();

			PopulateDebugMessengerCreateInfo(debugCreateInfo);
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
		}
		else
		{
			createInfo.enabledLayerCount = 0;
			createInfo.pNext = nullptr;
		}

		if (vkCreateInstance(&createInfo, nullptr, &m_VkInstance) != VK_SUCCESS)
		{
			ASSERT_MSG(false, "failed to create instance!");
		}
	}

	bool VulkanRenderer::CheckValidationLayerSupport()
	{
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char* layerName : m_ValidationLayers)
		{
			bool layerFound = false;

			for (const auto& layerProperties : availableLayers)
			{
				if (strcmp(layerName, layerProperties.layerName) == 0)
				{
					layerFound = true;
					break;
				}
			}

			if (!layerFound)
			{
				return false;
			}
		}

		return true;
	}

	std::vector<const char*> VulkanRenderer::GetRequiredExtensions()
	{
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		if (m_EnableValidationLayers)
		{
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		return extensions;
	}

	void VulkanRenderer::PrintExtensions()
	{
		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> extensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

		std::cout << "available extensions:" << std::endl;
		for (const auto& extension : extensions)
		{
			std::cout << "\t" << extension.extensionName << std::endl;
		}
	}

	void VulkanRenderer::SetupDebugMessenger()
	{
		if (!m_EnableValidationLayers)
			return;
	
		VkDebugUtilsMessengerCreateInfoEXT createInfo{};
		PopulateDebugMessengerCreateInfo(createInfo);
	
		if (VulkanProxy::CreateDebugUtilsMessenger(m_VkInstance, &createInfo, nullptr, &m_VkDebugMessenger) != VK_SUCCESS)
		{
			ASSERT_MSG(false, "failed to set up debug messenger!");
		}
	}

	void VulkanRenderer::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
	{
		createInfo = { VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = DebugCallback;
	}

	void VulkanRenderer::CreateRenderPass()
	{
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = m_pSwapChain->GetImageFormat();
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentDescription depthAttachment{};
		depthAttachment.format = FindDepthFormat();
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
		VkRenderPassCreateInfo renderPassInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(m_pDevice->GetDevice(), &renderPassInfo, nullptr, &m_VkRenderPass) != VK_SUCCESS)
		{
			ASSERT_MSG(false, "failed to create render pass!");
		}
	}

	void VulkanRenderer::CreateDescriptorSetLayout()
	{
		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		uboLayoutBinding.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutBinding samplerLayoutBinding{};
		samplerLayoutBinding.binding = 1;
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding.pImmutableSamplers = nullptr;
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };

		VkDescriptorSetLayoutCreateInfo layoutInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();

		if (vkCreateDescriptorSetLayout(m_pDevice->GetDevice(), &layoutInfo, nullptr, &m_VkDescriptorSetLayout) != VK_SUCCESS)
		{
			ASSERT_MSG(false, "failed to create descriptor set layout");
		}
	}

	void VulkanRenderer::CreateGraphicsPipeline()
	{
		VulkanShader* pShader = new VulkanShader("res/shaders/vert.spv", "res/shaders/frag.spv");

		auto bindingDescription = Vertex::GetBindingDescription();
		auto attributeDescriptions = Vertex::GetAttributeDescriptions();

		VkPipelineVertexInputStateCreateInfo vertexInputInfo = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssembly = { VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(m_pSwapChain->GetExtent().width);
		viewport.height = static_cast<float>(m_pSwapChain->GetExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = m_pSwapChain->GetExtent();

		VkPipelineViewportStateCreateInfo viewportState = { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		VkPipelineRasterizationStateCreateInfo rasterizer = { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		// rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.cullMode = VK_CULL_MODE_NONE;
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f;
		rasterizer.depthBiasClamp = 0.0f;
		rasterizer.depthBiasSlopeFactor = 0.0f;

		VkPipelineMultisampleStateCreateInfo multisampling = { VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading = 1.0f;
		multisampling.pSampleMask = nullptr;
		multisampling.alphaToCoverageEnable = VK_FALSE;
		multisampling.alphaToOneEnable = VK_FALSE;

		VkPipelineDepthStencilStateCreateInfo depthStencil = { VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
		depthStencil.depthTestEnable = VK_TRUE;
		depthStencil.depthWriteEnable = VK_TRUE;
		depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.minDepthBounds = 0.0f;
		depthStencil.maxDepthBounds = 1.0f;
		depthStencil.stencilTestEnable = VK_FALSE;
		depthStencil.front = {};
		depthStencil.back = {};

		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

		VkPipelineColorBlendStateCreateInfo colorBlending = { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;

		VkPipelineLayoutCreateInfo pipelineLayoutInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &m_VkDescriptorSetLayout;
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = nullptr;

		if (vkCreatePipelineLayout(m_pDevice->GetDevice(), &pipelineLayoutInfo, nullptr, &m_VkPipelineLayout) != VK_SUCCESS)
		{
			ASSERT_MSG(false, "failed to create pipeline layout!");
		}

		VkGraphicsPipelineCreateInfo pipelineInfo = { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
		std::vector<VkPipelineShaderStageCreateInfo> stages = pShader->GetShaderStages();
		pipelineInfo.stageCount = static_cast<uint32_t>(stages.size());
		pipelineInfo.pStages = stages.data();

		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = &depthStencil;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = nullptr;

		pipelineInfo.layout = m_VkPipelineLayout;
		pipelineInfo.renderPass = m_VkRenderPass;
		pipelineInfo.subpass = 0;

		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineInfo.basePipelineIndex = -1;

		if (vkCreateGraphicsPipelines(m_pDevice->GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_VkGraphicsPipeline) != VK_SUCCESS)
		{
			ASSERT_MSG(false, "failed to create graphics pipeline!");
		}

		delete pShader;
		pShader = nullptr;
	}

	void VulkanRenderer::CreateFramebuffers()
	{
		m_VkSwapChainFramebuffers.resize(m_pSwapChain->GetImageViews().size());

		for (size_t i = 0; i < m_pSwapChain->GetImageViews().size(); i++)
		{
			std::array<VkImageView, 2> attachments = {
				m_pSwapChain->GetImageViews()[i],
				m_VkDepthImageView
			};

			VkFramebufferCreateInfo framebufferInfo = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
			framebufferInfo.renderPass = m_VkRenderPass;
			framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = m_pSwapChain->GetExtent().width;
			framebufferInfo.height = m_pSwapChain->GetExtent().height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(m_pDevice->GetDevice(), &framebufferInfo, nullptr, &m_VkSwapChainFramebuffers[i]) != VK_SUCCESS)
			{
				ASSERT_MSG(false, "failed to create framebuffer!");
			}
		}
	}

	void VulkanRenderer::CreateCommandPool()
	{
		QueueFamilyIndices queueFamilyIndices = m_pDevice->FindQueueFamilies();

		VkCommandPoolCreateInfo poolInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		if (vkCreateCommandPool(m_pDevice->GetDevice(), &poolInfo, nullptr, &m_VkCommandPool) != VK_SUCCESS)
		{
			ASSERT_MSG(false, "failed to create command pool!");
		}
	}

	void VulkanRenderer::CreateDepthResources()
	{
		VkFormat depthFormat = FindDepthFormat();

		CreateImage(m_pSwapChain->GetExtent().width, m_pSwapChain->GetExtent().height, depthFormat, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			m_VkDepthImage, m_VkDepthImageMemory);
		m_VkDepthImageView = CreateImageView(m_VkDepthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

		TransitionImageLayout(m_VkDepthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
	}

	void VulkanRenderer::CreateUniformBuffers()
	{
		VkDeviceSize bufferSize = sizeof(UniformBufferObject);

		m_VkUniformBuffers.resize(m_pSwapChain->GetImages().size());
		m_VkUniformBuffersMemory.resize(m_pSwapChain->GetImages().size());

		for (size_t i = 0; i < m_pSwapChain->GetImages().size(); i++)
		{
			VulkanHelpers::CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				m_VkUniformBuffers[i], m_VkUniformBuffersMemory[i]);
		}
	}

	void VulkanRenderer::CreateDescriptorPool()
	{
		std::array<VkDescriptorPoolSize, 2> poolSizes{};

		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = static_cast<uint32_t>(m_pSwapChain->GetImages().size());

		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = static_cast<uint32_t>(m_pSwapChain->GetImages().size());

		VkDescriptorPoolCreateInfo poolInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = static_cast<uint32_t>(m_pSwapChain->GetImages().size());

		if (vkCreateDescriptorPool(m_pDevice->GetDevice(), &poolInfo, nullptr, &m_VkDescriptorPool) != VK_SUCCESS)
		{
			ASSERT_MSG(false, "failed to create descriptor pool!");
		}
	}

	void VulkanRenderer::CreateDescriptorSets()
	{
		std::vector<VkDescriptorSetLayout> layouts(m_pSwapChain->GetImages().size(), m_VkDescriptorSetLayout);
		VkDescriptorSetAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
		allocInfo.descriptorPool = m_VkDescriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(m_pSwapChain->GetImages().size());
		allocInfo.pSetLayouts = layouts.data();

		m_VkDescriptorSets.resize(m_pSwapChain->GetImages().size());
		if (vkAllocateDescriptorSets(m_pDevice->GetDevice(), &allocInfo, m_VkDescriptorSets.data()) != VK_SUCCESS)
		{
			ASSERT_MSG(false, "failed to allocate descriptor sets!");
		}

		for (size_t i = 0; i < m_pSwapChain->GetImages().size(); i++)
		{
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = m_VkUniformBuffers[i];
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(UniformBufferObject);

			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = m_pTexture->GetImageView();
			imageInfo.sampler = m_pTexture->GetSampler();

			std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = m_VkDescriptorSets[i];
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &bufferInfo;

			descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[1].dstSet = m_VkDescriptorSets[i];
			descriptorWrites[1].dstBinding = 1;
			descriptorWrites[1].dstArrayElement = 0;
			descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[1].descriptorCount = 1;
			descriptorWrites[1].pImageInfo = &imageInfo;

			vkUpdateDescriptorSets(m_pDevice->GetDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		}
	}

	void VulkanRenderer::CreateCommandBuffers()
	{
		m_VkCommandBuffers.resize(m_VkSwapChainFramebuffers.size());

		VkCommandBufferAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
		allocInfo.commandPool = m_VkCommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = static_cast<uint32_t>(m_VkCommandBuffers.size());

		if (vkAllocateCommandBuffers(m_pDevice->GetDevice(), &allocInfo, m_VkCommandBuffers.data()) != VK_SUCCESS)
		{
			ASSERT_MSG(false, "failed to allocate command buffers!");
		}
	}

	void VulkanRenderer::CreateSyncObjects()
	{
		m_VkImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_VkRenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_VkInFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
		m_VkImagesInFlight.resize(m_pSwapChain->GetImages().size(), VK_NULL_HANDLE);

		VkSemaphoreCreateInfo semaphoreInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };

		VkFenceCreateInfo fenceInfo = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			if (vkCreateSemaphore(m_pDevice->GetDevice(), &semaphoreInfo, nullptr, &m_VkImageAvailableSemaphores[i]) != VK_SUCCESS ||
				vkCreateSemaphore(m_pDevice->GetDevice(), &semaphoreInfo, nullptr, &m_VkRenderFinishedSemaphores[i]) != VK_SUCCESS ||
				vkCreateFence(m_pDevice->GetDevice(), &fenceInfo, nullptr, &m_VkInFlightFences[i]) != VK_SUCCESS)
			{
				ASSERT_MSG(false, "failed to create synchronization objects for a frame!");
			}
		}
	}

	void VulkanRenderer::CleanupSwapChain()
	{
		vkDestroyImageView(m_pDevice->GetDevice(), m_VkDepthImageView, nullptr);
		vkDestroyImage(m_pDevice->GetDevice(), m_VkDepthImage, nullptr);
		vkFreeMemory(m_pDevice->GetDevice(), m_VkDepthImageMemory, nullptr);

		for (size_t i = 0; i < m_VkSwapChainFramebuffers.size(); i++)
		{
			vkDestroyFramebuffer(m_pDevice->GetDevice(), m_VkSwapChainFramebuffers[i], nullptr);
		}

		vkFreeCommandBuffers(m_pDevice->GetDevice(), m_VkCommandPool, static_cast<uint32_t>(m_VkCommandBuffers.size()), m_VkCommandBuffers.data());

		vkDestroyPipeline(m_pDevice->GetDevice(), m_VkGraphicsPipeline, nullptr);
		vkDestroyPipelineLayout(m_pDevice->GetDevice(), m_VkPipelineLayout, nullptr);
		vkDestroyRenderPass(m_pDevice->GetDevice(), m_VkRenderPass, nullptr);

		m_pSwapChain->Cleanup();

		for (size_t i = 0; i < m_pSwapChain->GetImages().size(); i++)
		{
			vkDestroyBuffer(m_pDevice->GetDevice(), m_VkUniformBuffers[i], nullptr);
			vkFreeMemory(m_pDevice->GetDevice(), m_VkUniformBuffersMemory[i], nullptr);
		}

		vkDestroyDescriptorPool(m_pDevice->GetDevice(), m_VkDescriptorPool, nullptr);
	}

	void VulkanRenderer::RecreateSwapChain()
	{
		int width = 0, height = 0;
		glfwGetFramebufferSize(Application::Get().GetWindow()->GetGLFWWindow(), &width, &height);
		while (width == 0 || height == 0)
		{
			glfwGetFramebufferSize(Application::Get().GetWindow()->GetGLFWWindow(), &width, &height);
			glfwWaitEvents();
		}

		std::cout << "framebuffer resized, recreating swap chain!" << std::endl;

		vkDeviceWaitIdle(m_pDevice->GetDevice());

		CleanupSwapChain();

		m_pSwapChain->Initialize();

		CreateRenderPass();
		CreateGraphicsPipeline();
		CreateDepthResources();
		CreateFramebuffers();
		CreateUniformBuffers();
		CreateDescriptorPool();
		CreateDescriptorSets();
		CreateCommandBuffers();
	}

	void VulkanRenderer::UpdateUniformBuffer(uint32_t currentImage)
	{
		ASSERT_MSG(m_pCamera, "Current camera is nullptr!");

		UniformBufferObject ubo{};
		ubo.model = glm::mat4(1.0f); // just stay still for now
		ubo.view = m_pCamera->GetView();
		ubo.proj = m_pCamera->GetProjection();
		ubo.proj[1][1] *= -1;

		void* data;
		vkMapMemory(m_pDevice->GetDevice(), m_VkUniformBuffersMemory[currentImage], 0, sizeof(ubo), 0, &data);
			memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(m_pDevice->GetDevice(), m_VkUniformBuffersMemory[currentImage]);
	}

	void VulkanRenderer::CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
		VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
	{
		VkImageCreateInfo imageInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = usage;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.flags = 0;
		
		if (vkCreateImage(m_pDevice->GetDevice(), &imageInfo, nullptr, &image) != VK_SUCCESS)
		{
			ASSERT_MSG(false, "failed to create image!");
		}
		
		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(m_pDevice->GetDevice(), image, &memRequirements);
		
		VkMemoryAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = VulkanHelpers::FindMemoryType(memRequirements.memoryTypeBits, properties);
		
		if (vkAllocateMemory(m_pDevice->GetDevice(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
		{
			ASSERT_MSG(false, "failed to allocate image memory!");
		}
		
		vkBindImageMemory(m_pDevice->GetDevice(), image, imageMemory, 0);
	}

	void VulkanRenderer::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout,
		VkImageLayout newLayout)
	{
		VkCommandBuffer commandBuffer = VulkanHelpers::BeginSingleTimeCommands();
		
		VkImageMemoryBarrier barrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		
		VkPipelineStageFlags sourceStage{};
		VkPipelineStageFlags destinationStage{};
		
		if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		{
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		
			if (HasStencilComponent(format))
			{
				barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
			}
		}
		else
		{
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		}
		
		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		
			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		
			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		
			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		}
		else
		{
			ASSERT_MSG(false, "unsupported layout transition!");
		}
		
		vkCmdPipelineBarrier(
			commandBuffer,
			sourceStage, destinationStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);
		
		VulkanHelpers::EndSingleTimeCommands(commandBuffer);
	}
	
	void VulkanRenderer::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
	{
		VkCommandBuffer commandBuffer = VulkanHelpers::BeginSingleTimeCommands();
		
		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;
		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;
		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = { width, height, 1 };
		
		vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
		
		VulkanHelpers::EndSingleTimeCommands(commandBuffer);
	}
	
	VkImageView VulkanRenderer::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
	{
		VkImageViewCreateInfo viewInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
		viewInfo.image = image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;
	
		VkImageView imageView;
		if (vkCreateImageView(m_pDevice->GetDevice(), &viewInfo, nullptr, &imageView) != VK_SUCCESS)
		{
			ASSERT_MSG(false, "failed to create texture image view!");
		}
	
		return imageView;
	}

	VkFormat VulkanRenderer::FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling,
		VkFormatFeatureFlags features)
	{
		for (VkFormat format : candidates)
		{
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(m_pDevice->GetPhysicalDevice(), format, &props);

			if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
			{
				return format;
			}
			else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
			{
				return format;
			}
		}

		ASSERT_MSG(false, "failed to find supported format!");
		return candidates[0];
	}

	VkFormat VulkanRenderer::FindDepthFormat()
	{
		return FindSupportedFormat(
			{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
		);
	}

	bool VulkanRenderer::HasStencilComponent(VkFormat format)
	{
		return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
	}

	void VulkanRenderer::BeginCommandBuffers()
	{
		VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		beginInfo.flags = 0;
		beginInfo.pInheritanceInfo = nullptr;

		if (vkBeginCommandBuffer(m_VkCommandBuffers[m_CurrentBuffer], &beginInfo) != VK_SUCCESS)
		{
			ASSERT_MSG(false, "failed to begin recording command buffer!");
		}

		VkRenderPassBeginInfo renderPassInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
		renderPassInfo.renderPass = m_VkRenderPass;
		renderPassInfo.framebuffer = m_VkSwapChainFramebuffers[m_CurrentBuffer];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = m_pSwapChain->GetExtent();

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { {0.0f,0.0f,0.0f,1.0f} };
		clearValues[1].depthStencil = { 1.0f, 0 };

		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(m_VkCommandBuffers[m_CurrentBuffer], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
			vkCmdBindPipeline(m_VkCommandBuffers[m_CurrentBuffer], VK_PIPELINE_BIND_POINT_GRAPHICS, m_VkGraphicsPipeline);

			vkCmdBindDescriptorSets(m_VkCommandBuffers[m_CurrentBuffer], VK_PIPELINE_BIND_POINT_GRAPHICS, m_VkPipelineLayout, 0, 1,
				&m_VkDescriptorSets[m_CurrentBuffer], 0, nullptr);
	}

	void VulkanRenderer::EndCommandBuffers()
	{
		vkCmdEndRenderPass(m_VkCommandBuffers[m_CurrentBuffer]);

		if (vkEndCommandBuffer(m_VkCommandBuffers[m_CurrentBuffer]) != VK_SUCCESS)
		{
			ASSERT_MSG(false, "failed to record command buffer!");
		}
	}

	VkBool32 VulkanRenderer::DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT, VkDebugUtilsMessageTypeFlagsEXT,
	                                       const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void*)
	{
		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

		return VK_FALSE;
	}

	std::vector<char> VulkanRenderer::ReadFile(const std::string& filename)
	{
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open())
		{
			ASSERT_MSG(false, "failed to open file!");
		}

		size_t fileSize = static_cast<size_t>(file.tellg());
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);
		file.close();

		return buffer;
	}
}
