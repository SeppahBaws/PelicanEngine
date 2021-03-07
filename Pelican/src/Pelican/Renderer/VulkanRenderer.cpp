#include "PelicanPCH.h"
#include "VulkanRenderer.h"
#include "VulkanHelpers.h"

#include <logtools.h>

#include <fstream>
#include <cstdint>
#include <array>
#include <chrono>

#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "Pelican/Core/Application.h"

#include "VkInit.h"
#include "VulkanDebug.h"
#include "UniformBufferObject.h"
#include "Camera.h"

#include "Vertex.h"
#include "VulkanShader.h"
#include "VulkanHelpers.h"
#include "ImGui/ImGuiWrapper.h"


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

		if (m_EnableValidationLayers)
		{
			VkDebug::Setup(m_VkInstance);
		}

		m_pDevice = new VulkanDevice(m_VkInstance);

		if (m_EnableValidationLayers)
		{
			VkDebugMarker::Setup(m_pDevice->GetDevice());
		}

		m_pSwapChain = new VulkanSwapChain(m_pDevice);

		CreateRenderPass();
		CreateDescriptorSetLayout();
		CreateGraphicsPipeline();
		CreateCommandPool();
		CreateDepthResources();
		m_pSwapChain->CreateFramebuffers(m_VkDepthImageView, m_VkRenderPass);
		CreateUniformBuffers();
		CreateDescriptorPool();
		CreateCommandBuffers();
		CreateSyncObjects();

		m_pImGui = new ImGuiWrapper();

		ImGuiInitInfo imGuiInit = {};
		imGuiInit.instance = m_VkInstance;
		imGuiInit.physicalDevice = m_pDevice->GetPhysicalDevice();
		imGuiInit.device = m_pDevice->GetDevice();
		imGuiInit.queue = GetGraphicsQueue();
		imGuiInit.renderPass = m_VkRenderPass;
		m_pImGui->Init(imGuiInit);
	}

	void VulkanRenderer::BeforeSceneCleanup()
	{
		m_pDevice->WaitIdle();

		CleanupSwapChain();
	}

	void VulkanRenderer::AfterSceneCleanup()
	{
		m_pImGui->Cleanup();
		delete m_pImGui;
		m_pImGui = nullptr;

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
			VkDebug::FreeDebugCallback(m_VkInstance);
		}

		vkDestroyInstance(m_VkInstance, nullptr);
	}

	bool VulkanRenderer::BeginScene()
	{
		vkWaitForFences(m_pDevice->GetDevice(), 1, &m_VkInFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);

		VkResult result = vkAcquireNextImageKHR(m_pDevice->GetDevice(), m_pSwapChain->GetSwapChain(), UINT64_MAX, m_VkImageAvailableSemaphores[m_CurrentFrame], VK_NULL_HANDLE, &m_CurrentBuffer);

		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			RecreateSwapChain();
			return false;
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

		m_pImGui->NewFrame();

		return true;
	}

	void VulkanRenderer::EndScene()
	{
		m_pImGui->Render(m_VkCommandBuffers[m_CurrentBuffer]);

		// Submit our main scene rendering commands.
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

		VK_CHECK(vkQueueSubmit(m_pDevice->GetGraphicsQueue(), 1, &submitInfo, m_VkInFlightFences[m_CurrentFrame]));

		// Present the image to the window
		VkPresentInfoKHR presentInfo = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { m_pSwapChain->GetSwapChain() };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &m_CurrentBuffer;
		presentInfo.pResults = nullptr;

		VkResult result = vkQueuePresentKHR(m_pDevice->GetPresentQueue(), &presentInfo);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_FrameBufferResized)
		{
			m_FrameBufferResized = false;
			RecreateSwapChain();
		}
		else if (result != VK_SUCCESS)
		{
			ASSERT_MSG(false, "failed to present swap chain image!");
		}

		m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

		if (m_ReloadShadersFlag)
		{
			m_ReloadShadersFlag = false;
			ReloadShaders_Internal();
		}
	}

	void VulkanRenderer::SetCamera(Camera* pCamera)
	{
		m_pCamera = pCamera;
	}

	void VulkanRenderer::ReloadShaders()
	{
		m_ReloadShadersFlag = true;
	}

	void VulkanRenderer::CreateInstance()
	{
		if (m_EnableValidationLayers && !CheckValidationLayerSupport())
		{
			ASSERT_MSG(false, "validation layers requested, but not available!");
		}

		VkApplicationInfo appInfo = VkInit::ApplicationInfo();
		appInfo.pApplicationName = "Pelican Game";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "Pelican Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_2;

		// Extensions
		const std::vector<const char*> extensions = GetRequiredExtensions();
		VkInstanceCreateInfo createInfo = VkInit::InstanceCreateInfo(&appInfo, extensions);

		PrintExtensions();

		// Layers
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		if (m_EnableValidationLayers)
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(m_ValidationLayers.size());
			createInfo.ppEnabledLayerNames = m_ValidationLayers.data();

			debugCreateInfo = VkInit::DebugUtilsMessengerCreateInfo(VkDebug::DebugCallback);
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
		}
		else
		{
			createInfo.enabledLayerCount = 0;
			createInfo.pNext = nullptr;
		}

		VK_CHECK(vkCreateInstance(&createInfo, nullptr, &m_VkInstance));
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
			extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
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
		VkRenderPassCreateInfo renderPassInfo = VkInit::RenderPassCreateInfo();
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		VK_CHECK(vkCreateRenderPass(m_pDevice->GetDevice(), &renderPassInfo, nullptr, &m_VkRenderPass));

		VkDebugMarker::SetRenderPassName(m_pDevice->GetDevice(), m_VkRenderPass, "Main Render Pass");
	}

	void VulkanRenderer::CreateDescriptorSetLayout()
	{
		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		uboLayoutBinding.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutBinding albedoSamplerBinding{};
		albedoSamplerBinding.binding = 1;
		albedoSamplerBinding.descriptorCount = 1;
		albedoSamplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		albedoSamplerBinding.pImmutableSamplers = nullptr;
		albedoSamplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		VkDescriptorSetLayoutBinding normalSamplerBinding{};
		normalSamplerBinding.binding = 2;
		normalSamplerBinding.descriptorCount = 1;
		normalSamplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		normalSamplerBinding.pImmutableSamplers = nullptr;
		normalSamplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		VkDescriptorSetLayoutBinding metallicRoughnessSamplerBinding{};
		metallicRoughnessSamplerBinding.binding = 3;
		metallicRoughnessSamplerBinding.descriptorCount = 1;
		metallicRoughnessSamplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		metallicRoughnessSamplerBinding.pImmutableSamplers = nullptr;
		metallicRoughnessSamplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		std::array<VkDescriptorSetLayoutBinding, 4> bindings = {
			uboLayoutBinding,
			albedoSamplerBinding,
			normalSamplerBinding,
			metallicRoughnessSamplerBinding
		};

		VkDescriptorSetLayoutCreateInfo layoutInfo = VkInit::DescriptorSetLayoutCreateInfo();
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();

		VK_CHECK(vkCreateDescriptorSetLayout(m_pDevice->GetDevice(), &layoutInfo, nullptr, &m_VkDescriptorSetLayout));
	}

	void VulkanRenderer::CreateGraphicsPipeline()
	{
		VulkanShader* pShader = new VulkanShader("res/shaders/vert.spv", "res/shaders/frag.spv");

		auto bindingDescription = Vertex::GetBindingDescription();
		auto attributeDescriptions = Vertex::GetAttributeDescriptions();

		VkPipelineVertexInputStateCreateInfo vertexInputInfo = VkInit::VertexInputStateCreateInfo();
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssembly = VkInit::InputAssemblyStateCreateInfo(
			VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

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

		VkPipelineRasterizationStateCreateInfo rasterizer = VkInit::RasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT);
		VkPipelineMultisampleStateCreateInfo multisampling = VkInit::MultisampleStateCreateInfo();
		VkPipelineDepthStencilStateCreateInfo depthStencil = VkInit::DepthStencilCreateInfo(true, true, VK_COMPARE_OP_LESS);
		VkPipelineColorBlendAttachmentState colorBlendAttachment = VkInit::ColorBlendAttachmentState();
		VkPipelineColorBlendStateCreateInfo colorBlending = VkInit::ColorBlendState();
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;

		VkPipelineLayoutCreateInfo pipelineLayoutInfo = VkInit::PipelineLayoutCreateInfo();
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &m_VkDescriptorSetLayout;

		VK_CHECK(vkCreatePipelineLayout(m_pDevice->GetDevice(), &pipelineLayoutInfo, nullptr, &m_VkPipelineLayout));

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

		VK_CHECK(vkCreateGraphicsPipelines(m_pDevice->GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_VkGraphicsPipeline));

		delete pShader;
		pShader = nullptr;
	}

	void VulkanRenderer::CreateCommandPool()
	{
		QueueFamilyIndices queueFamilyIndices = m_pDevice->FindQueueFamilies();

		VkCommandPoolCreateInfo poolInfo = VkInit::CommandPoolCreateInfo(queueFamilyIndices.graphicsFamily.value(),
			VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

		VK_CHECK(vkCreateCommandPool(m_pDevice->GetDevice(), &poolInfo, nullptr, &m_VkCommandPool));
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

		VK_CHECK(vkCreateDescriptorPool(m_pDevice->GetDevice(), &poolInfo, nullptr, &m_VkDescriptorPool));
	}

	void VulkanRenderer::CreateCommandBuffers()
	{
		m_VkCommandBuffers.resize(m_pSwapChain->GetFramebuffers().size());

		VkCommandBufferAllocateInfo allocInfo = VkInit::CommandBufferAllocateInfo(m_VkCommandPool);
		allocInfo.commandBufferCount = static_cast<uint32_t>(m_VkCommandBuffers.size());

		VK_CHECK(vkAllocateCommandBuffers(m_pDevice->GetDevice(), &allocInfo, m_VkCommandBuffers.data()));
	}

	void VulkanRenderer::CreateSyncObjects()
	{
		m_VkImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_VkRenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_VkInFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
		m_VkImagesInFlight.resize(m_pSwapChain->GetImages().size(), VK_NULL_HANDLE);

		VkSemaphoreCreateInfo semaphoreInfo = VkInit::SemaphoreCreateInfo();
		VkFenceCreateInfo fenceInfo = VkInit::FenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			VK_CHECK(vkCreateSemaphore(m_pDevice->GetDevice(), &semaphoreInfo, nullptr, &m_VkImageAvailableSemaphores[i]));
			VK_CHECK(vkCreateSemaphore(m_pDevice->GetDevice(), &semaphoreInfo, nullptr, &m_VkRenderFinishedSemaphores[i]));
			VK_CHECK(vkCreateFence(m_pDevice->GetDevice(), &fenceInfo, nullptr, &m_VkInFlightFences[i]));
		}
	}

	void VulkanRenderer::CleanupSwapChain()
	{
		vkDestroyImageView(m_pDevice->GetDevice(), m_VkDepthImageView, nullptr);
		vkDestroyImage(m_pDevice->GetDevice(), m_VkDepthImage, nullptr);
		vkFreeMemory(m_pDevice->GetDevice(), m_VkDepthImageMemory, nullptr);

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
		Window::Params params = Application::Get().GetWindow()->GetParams();
		while (params.width == 0 || params.height == 0)
		{
			params = Application::Get().GetWindow()->GetParams();
			glfwWaitEvents();
		}

		// Passing 0 to not change those parameters
		m_pCamera->UpdateProjection(0, static_cast<float>(params.width), static_cast<float>(params.height), 0, 0);

		Logger::LogTrace("Framebuffer resized, recreating swap chain!");

		vkDeviceWaitIdle(m_pDevice->GetDevice());

		CleanupSwapChain();

		m_pSwapChain->Initialize();

		CreateRenderPass();
		CreateGraphicsPipeline();
		CreateDepthResources();
		m_pSwapChain->CreateFramebuffers(m_VkDepthImageView, m_VkRenderPass);
		CreateUniformBuffers();
		CreateDescriptorPool();
		CreateCommandBuffers();
	}

	void VulkanRenderer::ReloadShaders_Internal()
	{
		vkDeviceWaitIdle(m_pDevice->GetDevice());

		// Cleanup pipeline
		vkDestroyPipeline(m_pDevice->GetDevice(), m_VkGraphicsPipeline, nullptr);
		vkDestroyPipelineLayout(m_pDevice->GetDevice(), m_VkPipelineLayout, nullptr);
		vkDestroyRenderPass(m_pDevice->GetDevice(), m_VkRenderPass, nullptr);

		CreateRenderPass();
		CreateGraphicsPipeline();
	}

	void VulkanRenderer::UpdateUniformBuffer(uint32_t currentImage)
	{
		ASSERT_MSG(m_pCamera, "Current camera is nullptr!");

		UniformBufferObject ubo{};
		ubo.model = glm::mat4(1.0f); // just stay still for now
		ubo.model = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)) *
			glm::scale(glm::mat4(1.0f), glm::vec3(0.1f, 0.1f, 0.1f));
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
		VkImageCreateInfo imageInfo = VkInit::ImageCreateInfo(VK_IMAGE_TYPE_2D);
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.usage = usage;
		
		VK_CHECK(vkCreateImage(m_pDevice->GetDevice(), &imageInfo, nullptr, &image));
		
		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(m_pDevice->GetDevice(), image, &memRequirements);
		
		VkMemoryAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = VulkanHelpers::FindMemoryType(memRequirements.memoryTypeBits, properties);
		
		VK_CHECK(vkAllocateMemory(m_pDevice->GetDevice(), &allocInfo, nullptr, &imageMemory));
		
		vkBindImageMemory(m_pDevice->GetDevice(), image, imageMemory, 0);
	}

	void VulkanRenderer::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout,
		VkImageLayout newLayout)
	{
		VkCommandBuffer commandBuffer = VulkanHelpers::BeginSingleTimeCommands();
		
		VkImageMemoryBarrier barrier = VkInit::ImageMemoryBarrier();
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.image = image;
		
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
		VkImageViewCreateInfo viewInfo = VkInit::ImageViewCreateInfo(image, VK_IMAGE_VIEW_TYPE_2D, format);
		viewInfo.subresourceRange.aspectMask = aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;
	
		VkImageView imageView;
		VK_CHECK(vkCreateImageView(m_pDevice->GetDevice(), &viewInfo, nullptr, &imageView));
	
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
		VkCommandBufferBeginInfo beginInfo = VkInit::CommandBufferBeginInfo();

		VK_CHECK(vkBeginCommandBuffer(m_VkCommandBuffers[m_CurrentBuffer], &beginInfo));

		VkRenderPassBeginInfo renderPassInfo = VkInit::RenderPassBeginInfo();
		renderPassInfo.renderPass = m_VkRenderPass;
		renderPassInfo.framebuffer = m_pSwapChain->GetFramebuffers()[m_CurrentBuffer];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = m_pSwapChain->GetExtent();

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { {0.1f,0.1f,0.1f,1.0f} };
		clearValues[1].depthStencil = { 1.0f, 0 };

		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(m_VkCommandBuffers[m_CurrentBuffer], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
			vkCmdBindPipeline(m_VkCommandBuffers[m_CurrentBuffer], VK_PIPELINE_BIND_POINT_GRAPHICS, m_VkGraphicsPipeline);
	}

	void VulkanRenderer::EndCommandBuffers()
	{
		vkCmdEndRenderPass(m_VkCommandBuffers[m_CurrentBuffer]);

		VK_CHECK(vkEndCommandBuffer(m_VkCommandBuffers[m_CurrentBuffer]));
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
