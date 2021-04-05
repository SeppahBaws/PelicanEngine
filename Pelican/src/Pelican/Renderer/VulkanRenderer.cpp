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
			VkDebug::Setup(m_Instance.get());
		}

		m_pDevice = new VulkanDevice(m_Instance.get());

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
		m_pSwapChain->CreateFramebuffers(m_DepthImageView, m_RenderPass);
		CreateUniformBuffers();
		CreateDescriptorPool();
		CreateCommandBuffers();
		CreateSyncObjects();

		m_pImGui = new ImGuiWrapper();

		ImGuiInitInfo imGuiInit = {};
		imGuiInit.instance = m_Instance.get();
		imGuiInit.physicalDevice = m_pDevice->GetPhysicalDevice();
		imGuiInit.device = m_pDevice->GetDevice();
		imGuiInit.queue = GetGraphicsQueue();
		imGuiInit.renderPass = m_RenderPass;
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

		m_pDevice->GetDevice().destroyDescriptorSetLayout(m_DescriptorSetLayout);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			m_pDevice->GetDevice().destroySemaphore(m_RenderFinishedSemaphores[i]);
			m_pDevice->GetDevice().destroySemaphore(m_ImageAvailableSemaphores[i]);
			m_pDevice->GetDevice().destroyFence(m_InFlightFences[i]);
		}

		m_pDevice->GetDevice().destroyCommandPool(m_CommandPool);

		delete m_pSwapChain;
		m_pSwapChain = nullptr;

		delete m_pDevice;
		m_pDevice = nullptr;

		if (m_EnableValidationLayers)
		{
			VkDebug::FreeDebugCallback(m_Instance.get());
		}

		// vkDestroyInstance(m_Instance, nullptr);
	}

	bool VulkanRenderer::BeginScene()
	{
		vk::Result result = m_pDevice->GetDevice().waitForFences(m_InFlightFences[m_CurrentFrame], true, UINT64_MAX);
		if (result != vk::Result::eSuccess)
		{
			throw std::runtime_error("Failed to wait for fence");
		}

		result = m_pDevice->GetDevice().acquireNextImageKHR(
			m_pSwapChain->GetSwapChain(),
			// UINT64_MAX,
			1'000'000'000,
			m_ImageAvailableSemaphores[m_CurrentFrame],
			nullptr,
			&m_CurrentBuffer);

		if (result == vk::Result::eErrorOutOfDateKHR)
		{
			RecreateSwapChain();
			return false;
		}
		else if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR)
		{
			throw std::runtime_error("Failed to acquire swap chain image!");
		}

		if (m_ImagesInFlight[m_CurrentBuffer])
		{
			result = m_pDevice->GetDevice().waitForFences(m_ImagesInFlight[m_CurrentBuffer], true, UINT64_MAX);
			if (result != vk::Result::eSuccess)
				throw std::runtime_error("Failed to wait for fence");
		}
		m_ImagesInFlight[m_CurrentBuffer] = m_InFlightFences[m_CurrentFrame];

		UpdateUniformBuffer(m_CurrentBuffer);


		BeginCommandBuffers();

		m_pImGui->NewFrame();

		return true;
	}

	void VulkanRenderer::EndScene()
	{
		m_pImGui->Render(m_CommandBuffers[m_CurrentBuffer]);

		// Submit our main scene rendering commands.
		EndCommandBuffers();

		vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };

		const vk::SubmitInfo submitInfo = vk::SubmitInfo()
			.setWaitSemaphores(m_ImageAvailableSemaphores[m_CurrentFrame])
			.setPWaitDstStageMask(waitStages)
			.setCommandBuffers(m_CommandBuffers[m_CurrentBuffer])
			.setSignalSemaphores(m_RenderFinishedSemaphores[m_CurrentFrame]);

		m_pDevice->GetDevice().resetFences(m_InFlightFences[m_CurrentFrame]);

		try
		{
			m_pDevice->GetGraphicsQueue().submit(submitInfo, m_InFlightFences[m_CurrentFrame]);
		}
		catch (vk::SystemError& e)
		{
			throw std::runtime_error("Failed to submit to the graphics queue: "s + e.what());
		}

		// Present the image to the window
		std::vector<vk::SwapchainKHR> swapChains = { m_pSwapChain->GetSwapChain() };
		const vk::PresentInfoKHR presentInfo = vk::PresentInfoKHR()
			.setWaitSemaphores(m_RenderFinishedSemaphores[m_CurrentFrame])
			.setResults(nullptr)
			.setImageIndices(m_CurrentBuffer)
			.setSwapchains(swapChains);

		vk::Result result{};

		try
		{
			result = m_pDevice->GetPresentQueue().presentKHR(presentInfo);
		}
		catch (vk::OutOfDateKHRError& /*e*/)
		{
			m_FrameBufferResized = false;
			RecreateSwapChain();
		}
		catch (vk::SystemError& e)
		{
			throw std::runtime_error("Failed to present: "s + e.what());
		}

		if (result == vk::Result::eSuboptimalKHR || m_FrameBufferResized)
		{
			m_FrameBufferResized = false;
			RecreateSwapChain();
		}
		else if (result != vk::Result::eSuccess)
		{
			throw std::runtime_error("Failed to present swap chain image!");
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
			throw std::runtime_error("Validation layers were requested, but none are available!");
		}

		const vk::ApplicationInfo appInfo = vk::ApplicationInfo()
			.setPApplicationName("Pelican Game")
			.setApplicationVersion(VK_MAKE_VERSION(1, 0, 0))
			.setPEngineName("Pelican Engine")
			.setEngineVersion(VK_MAKE_VERSION(1, 0, 0))
			.setApiVersion(VK_MAKE_VERSION(1, 2, 0));

		const std::vector<const char*> extensions = GetRequiredExtensions();

		vk::InstanceCreateInfo createInfo = vk::InstanceCreateInfo()
			.setPApplicationInfo(&appInfo)
			.setPEnabledLayerNames({})
			.setPEnabledExtensionNames(extensions);

		PrintExtensions();

		// Layers
		if (m_EnableValidationLayers)
		{
			createInfo.setPEnabledLayerNames(m_ValidationLayers);
		}

		try
		{
			m_Instance = vk::createInstanceUnique(createInfo);
		}
		catch (vk::SystemError& e)
		{
			throw std::runtime_error("Failed to create instance: "s + e.what());
		}
	}

	bool VulkanRenderer::CheckValidationLayerSupport() const
	{
		std::vector<vk::LayerProperties> availableLayers = vk::enumerateInstanceLayerProperties();

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

	std::vector<const char*> VulkanRenderer::GetRequiredExtensions() const
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

	void VulkanRenderer::PrintExtensions() const
	{
		std::vector<vk::ExtensionProperties> extensions = vk::enumerateInstanceExtensionProperties();

		std::cout << "available extensions:" << std::endl;
		for (const auto& extension : extensions)
		{
			std::cout << "\t" << extension.extensionName << std::endl;
		}
	}

	void VulkanRenderer::CreateRenderPass()
	{
		const vk::AttachmentDescription colorAttachment = vk::AttachmentDescription()
			.setFormat(m_pSwapChain->GetImageFormat())
			.setSamples(vk::SampleCountFlagBits::e1)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

		const vk::AttachmentDescription depthAttachment = vk::AttachmentDescription()
			.setFormat(FindDepthFormat())
			.setSamples(vk::SampleCountFlagBits::e1)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

		const vk::AttachmentReference colorAttachmentRef = vk::AttachmentReference()
			.setAttachment(0)
			.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

		const vk::AttachmentReference depthAttachmentRef = vk::AttachmentReference()
			.setAttachment(1)
			.setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

		std::array<vk::AttachmentReference, 1> colorAttachmentRefs = { colorAttachmentRef };
		vk::SubpassDescription subpass = vk::SubpassDescription()
			.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
			.setColorAttachments(colorAttachmentRefs)
			.setPDepthStencilAttachment(&depthAttachmentRef);

		vk::SubpassDependency dependency = vk::SubpassDependency()
			.setSrcSubpass(VK_SUBPASS_EXTERNAL)
			.setDstSubpass(0)
			.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
			.setSrcAccessMask({})
			.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
			.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);

		std::array<vk::AttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
		const vk::RenderPassCreateInfo renderPassInfo = vk::RenderPassCreateInfo()
			.setAttachments(attachments)
			.setSubpasses(subpass)
			.setDependencies(dependency);

		try
		{
			m_RenderPass = m_pDevice->GetDevice().createRenderPass(renderPassInfo);
		}
		catch (vk::SystemError& e)
		{
			throw std::runtime_error("Failed to create render pass: "s + e.what());
		}

		VkDebugMarker::SetRenderPassName(m_pDevice->GetDevice(), m_RenderPass, "Main Render Pass");
	}

	void VulkanRenderer::CreateDescriptorSetLayout()
	{
		const vk::DescriptorSetLayoutBinding uboLayoutBinding = vk::DescriptorSetLayoutBinding()
			.setBinding(0)
			.setDescriptorType(vk::DescriptorType::eUniformBuffer)
			.setDescriptorCount(1)
			.setStageFlags(vk::ShaderStageFlagBits::eVertex)
			.setPImmutableSamplers(nullptr);

		const vk::DescriptorSetLayoutBinding albedoSamplerBinding = vk::DescriptorSetLayoutBinding()
			.setBinding(1)
			.setDescriptorCount(1)
			.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
			.setPImmutableSamplers(nullptr)
			.setStageFlags(vk::ShaderStageFlagBits::eFragment);

		const vk::DescriptorSetLayoutBinding normalSamplerBinding = vk::DescriptorSetLayoutBinding()
			.setBinding(2)
			.setDescriptorCount(1)
			.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
			.setPImmutableSamplers(nullptr)
			.setStageFlags(vk::ShaderStageFlagBits::eFragment);

		const vk::DescriptorSetLayoutBinding metallicRoughnessSamplerBinding = vk::DescriptorSetLayoutBinding()
			.setBinding(3)
			.setDescriptorCount(1)
			.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
			.setPImmutableSamplers(nullptr)
			.setStageFlags(vk::ShaderStageFlagBits::eFragment);

		std::array<vk::DescriptorSetLayoutBinding, 4> bindings = {
			uboLayoutBinding,
			albedoSamplerBinding,
			normalSamplerBinding,
			metallicRoughnessSamplerBinding
		};

		const vk::DescriptorSetLayoutCreateInfo layoutInfo = vk::DescriptorSetLayoutCreateInfo()
			.setBindings(bindings);

		try
		{
			m_DescriptorSetLayout = m_pDevice->GetDevice().createDescriptorSetLayout(layoutInfo);
		}
		catch (vk::SystemError& e)
		{
			throw std::runtime_error("failed to create descriptor set layout: "s + e.what());
		}
	}

	void VulkanRenderer::CreateGraphicsPipeline()
	{
		VulkanShader* pShader = new VulkanShader("res/shaders/vert.spv", "res/shaders/frag.spv");

		auto bindingDescription = Vertex::GetBindingDescription();
		auto attributeDescriptions = Vertex::GetAttributeDescriptions();

		const vk::PipelineVertexInputStateCreateInfo vertexInputInfo = vk::PipelineVertexInputStateCreateInfo()
			.setVertexBindingDescriptions(bindingDescription)
			.setVertexAttributeDescriptions(attributeDescriptions);

		const vk::PipelineInputAssemblyStateCreateInfo inputAssembly = vk::PipelineInputAssemblyStateCreateInfo()
			.setTopology(vk::PrimitiveTopology::eTriangleList)
			.setPrimitiveRestartEnable(false);

		vk::Viewport viewport = vk::Viewport()
			.setX(0.0f).setY(0.0f)
			.setWidth(static_cast<float>(m_pSwapChain->GetExtent().width))
			.setHeight(static_cast<float>(m_pSwapChain->GetExtent().height))
			.setMinDepth(0.0f).setMaxDepth(1.0f);

		vk::Rect2D scissor = vk::Rect2D()
			.setOffset(vk::Offset2D(0, 0))
			.setExtent(m_pSwapChain->GetExtent());

		const vk::PipelineViewportStateCreateInfo viewportState = vk::PipelineViewportStateCreateInfo()
			.setViewports(viewport)
			.setScissors(scissor);

		vk::PipelineRasterizationStateCreateInfo rasterizer = VkInit::RasterizationStateCreateInfo(vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack);
		vk::PipelineMultisampleStateCreateInfo multisampling = VkInit::MultisampleStateCreateInfo();
		vk::PipelineDepthStencilStateCreateInfo depthStencil = VkInit::DepthStencilCreateInfo(true, true, vk::CompareOp::eLess);
		vk::PipelineColorBlendAttachmentState colorBlendAttachment = vk::PipelineColorBlendAttachmentState()
			.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA)
			.setBlendEnable(true)
			.setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
			.setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
			.setColorBlendOp(vk::BlendOp::eAdd)
			.setSrcAlphaBlendFactor(vk::BlendFactor::eSrcAlpha)
			.setDstAlphaBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
			.setAlphaBlendOp(vk::BlendOp::eAdd);
		vk::PipelineColorBlendStateCreateInfo colorBlending = vk::PipelineColorBlendStateCreateInfo()
			.setLogicOpEnable(false)
			.setLogicOp(vk::LogicOp::eCopy)
			.setAttachments(colorBlendAttachment)
			.setBlendConstants({ 0.0f, 0.0f, 0.0f, 0.0f });

		vk::PipelineLayoutCreateInfo pipelineLayoutInfo = vk::PipelineLayoutCreateInfo()
			.setSetLayouts(m_DescriptorSetLayout);

		try
		{
			m_PipelineLayout = m_pDevice->GetDevice().createPipelineLayout(pipelineLayoutInfo);
		}
		catch (vk::SystemError& e)
		{
			throw std::runtime_error("Failed to create pipeline layout: "s + e.what());
		}

		std::vector<vk::PipelineShaderStageCreateInfo> stages = pShader->GetShaderStages();
		const vk::GraphicsPipelineCreateInfo pipelineInfo = vk::GraphicsPipelineCreateInfo()
			.setStages(stages)

			.setPVertexInputState(&vertexInputInfo)
			.setPInputAssemblyState(&inputAssembly)
			.setPViewportState(&viewportState)
			.setPRasterizationState(&rasterizer)
			.setPMultisampleState(&multisampling)
			.setPDepthStencilState(&depthStencil)
			.setPColorBlendState(&colorBlending)
			.setPDynamicState(nullptr)

			.setLayout(m_PipelineLayout)
			.setRenderPass(m_RenderPass)
			.setSubpass(0)

			.setBasePipelineHandle(nullptr)
			.setBasePipelineIndex(-1);

		m_PipelineCache = m_pDevice->GetDevice().createPipelineCache(vk::PipelineCacheCreateInfo());

		vk::ResultValue<vk::Pipeline> pipelineResult = m_pDevice->GetDevice().createGraphicsPipeline(m_PipelineCache, pipelineInfo);

		if (pipelineResult.result != vk::Result::eSuccess)
		{
			throw std::runtime_error("Failed to create graphics pipeline");
		}
		m_GraphicsPipeline = pipelineResult.value;

		delete pShader;
		pShader = nullptr;
	}

	void VulkanRenderer::CreateCommandPool()
	{
		QueueFamilyIndices queueFamilyIndices = m_pDevice->FindQueueFamilies();

		const vk::CommandPoolCreateInfo poolInfo = vk::CommandPoolCreateInfo()
			.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
			.setQueueFamilyIndex(queueFamilyIndices.graphicsFamily.value());

		try
		{
			m_CommandPool = m_pDevice->GetDevice().createCommandPool(poolInfo);
		}
		catch (vk::SystemError& e)
		{
			throw std::runtime_error("Failed to create command pool: "s + e.what());
		}
	}

	void VulkanRenderer::CreateDepthResources()
	{
		const vk::Format depthFormat = FindDepthFormat();

		CreateImage(m_pSwapChain->GetExtent().width, m_pSwapChain->GetExtent().height, depthFormat, vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal,
			m_DepthImage, m_DepthImageMemory);
		m_DepthImageView = CreateImageView(m_DepthImage, depthFormat, vk::ImageAspectFlagBits::eDepth);

		TransitionImageLayout(m_DepthImage, depthFormat, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal);
	}

	void VulkanRenderer::CreateUniformBuffers()
	{
		const vk::DeviceSize bufferSize = sizeof(UniformBufferObject);

		m_UniformBuffers.resize(m_pSwapChain->GetImages().size());
		m_UniformBuffersMemory.resize(m_pSwapChain->GetImages().size());

		for (size_t i = 0; i < m_pSwapChain->GetImages().size(); i++)
		{
			VulkanHelpers::CreateBuffer(
				bufferSize,
				vk::BufferUsageFlagBits::eUniformBuffer,
				vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
				m_UniformBuffers[i], m_UniformBuffersMemory[i]
			);
		}
	}

	void VulkanRenderer::CreateDescriptorPool()
	{
		std::array<vk::DescriptorPoolSize, 2> poolSizes{};

		poolSizes[0].type = vk::DescriptorType::eUniformBuffer;
		poolSizes[0].descriptorCount = static_cast<uint32_t>(m_pSwapChain->GetImages().size());

		poolSizes[1].type = vk::DescriptorType::eCombinedImageSampler;
		poolSizes[1].descriptorCount = static_cast<uint32_t>(m_pSwapChain->GetImages().size());

		const vk::DescriptorPoolCreateInfo poolInfo = vk::DescriptorPoolCreateInfo()
			.setMaxSets(static_cast<uint32_t>(m_pSwapChain->GetImages().size()))
			.setPoolSizes(poolSizes);

		try
		{
			m_DescriptorPool = m_pDevice->GetDevice().createDescriptorPool(poolInfo);
		}
		catch (vk::SystemError& e)
		{
			throw std::runtime_error("Failed to create descriptor pool: "s + e.what());
		}
	}

	void VulkanRenderer::CreateCommandBuffers()
	{
		m_CommandBuffers.resize(m_pSwapChain->GetFramebuffers().size());

		const vk::CommandBufferAllocateInfo allocInfo = vk::CommandBufferAllocateInfo()
			.setCommandPool(m_CommandPool)
			.setCommandBufferCount(static_cast<uint32_t>(m_CommandBuffers.size()))
			.setLevel(vk::CommandBufferLevel::ePrimary);

		try
		{
			m_CommandBuffers = m_pDevice->GetDevice().allocateCommandBuffers(allocInfo);
		}
		catch (vk::SystemError& e)
		{
			throw std::runtime_error("Failed to create command buffers: "s + e.what());
		}
	}

	void VulkanRenderer::CreateSyncObjects()
	{
		m_ImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_RenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_InFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
		m_ImagesInFlight.resize(m_pSwapChain->GetImages().size(), nullptr);

		const vk::SemaphoreCreateInfo semaphoreInfo = vk::SemaphoreCreateInfo();
		const vk::FenceCreateInfo fenceInfo = vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled);

		try
		{
			for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
			{
				m_ImageAvailableSemaphores[i] = m_pDevice->GetDevice().createSemaphore(semaphoreInfo);
				m_RenderFinishedSemaphores[i] = m_pDevice->GetDevice().createSemaphore(semaphoreInfo);
				m_InFlightFences[i] = m_pDevice->GetDevice().createFence(fenceInfo);
			}
		}
		catch (vk::SystemError& e)
		{
			throw std::runtime_error("Failed to create sync objects: "s + e.what());
		}
	}

	void VulkanRenderer::CleanupSwapChain()
	{
		m_pDevice->GetDevice().destroyImageView(m_DepthImageView);
		m_pDevice->GetDevice().destroyImage(m_DepthImage);
		m_pDevice->GetDevice().freeMemory(m_DepthImageMemory);

		m_pDevice->GetDevice().freeCommandBuffers(m_CommandPool, m_CommandBuffers);

		m_pDevice->GetDevice().destroyPipeline(m_GraphicsPipeline);
		m_pDevice->GetDevice().destroyPipelineCache(m_PipelineCache);
		m_pDevice->GetDevice().destroyPipelineLayout(m_PipelineLayout);
		m_pDevice->GetDevice().destroyRenderPass(m_RenderPass);

		m_pSwapChain->Cleanup();

		for (size_t i = 0; i < m_pSwapChain->GetImages().size(); i++)
		{
			m_pDevice->GetDevice().destroyBuffer(m_UniformBuffers[i]);
			m_pDevice->GetDevice().freeMemory(m_UniformBuffersMemory[i]);
		}

		m_pDevice->GetDevice().destroyDescriptorPool(m_DescriptorPool);
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

		m_pDevice->GetDevice().waitIdle();

		CleanupSwapChain();

		m_pSwapChain->Initialize();

		CreateRenderPass();
		CreateGraphicsPipeline();
		CreateDepthResources();
		m_pSwapChain->CreateFramebuffers(m_DepthImageView, m_RenderPass);
		CreateUniformBuffers();
		CreateDescriptorPool();
		CreateCommandBuffers();
	}

	void VulkanRenderer::ReloadShaders_Internal()
	{
		m_pDevice->WaitIdle();

		// Cleanup pipeline
		m_pDevice->GetDevice().destroyPipeline(m_GraphicsPipeline);
		// TODO: we should not have to destroy the pipeline cache and pipeline layout, for faster pipeline rebuilding.
		m_pDevice->GetDevice().destroyPipelineCache(m_PipelineCache);
		m_pDevice->GetDevice().destroyPipelineLayout(m_PipelineLayout);
		m_pDevice->GetDevice().destroyRenderPass(m_RenderPass);

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

		void* data = m_pDevice->GetDevice().mapMemory(m_UniformBuffersMemory[currentImage], 0, sizeof(ubo));
			memcpy(data, &ubo, sizeof(ubo));
		m_pDevice->GetDevice().unmapMemory(m_UniformBuffersMemory[currentImage]);
	}

	void VulkanRenderer::CreateImage(uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling,
		vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Image& image, vk::DeviceMemory& imageMemory) const
	{
		const vk::ImageCreateInfo imageInfo = vk::ImageCreateInfo()
			.setImageType(vk::ImageType::e2D)
			.setExtent(vk::Extent3D(width, height, 1))
			.setFormat(format)
			.setTiling(tiling)
			.setUsage(usage)
			.setMipLevels(1)
			.setArrayLayers(1)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setSharingMode(vk::SharingMode::eExclusive)
			.setSamples(vk::SampleCountFlagBits::e1);

		try
		{
			image = m_pDevice->GetDevice().createImage(imageInfo);
		}
		catch(vk::SystemError& e)
		{
			throw std::runtime_error("Failed to create image: "s + e.what());
		}

		const vk::MemoryRequirements memRequirements = m_pDevice->GetDevice().getImageMemoryRequirements(image);

		const vk::MemoryAllocateInfo allocInfo = vk::MemoryAllocateInfo()
			.setAllocationSize(memRequirements.size)
			.setMemoryTypeIndex(VulkanHelpers::FindMemoryType(memRequirements.memoryTypeBits, properties));

		imageMemory = m_pDevice->GetDevice().allocateMemory(allocInfo);

		m_pDevice->GetDevice().bindImageMemory(image, imageMemory, 0);
	}

	void VulkanRenderer::TransitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldLayout,
		vk::ImageLayout newLayout) const
	{
		const vk::CommandBuffer cmd = VulkanHelpers::BeginSingleTimeCommands();

		vk::ImageMemoryBarrier barrier = VkInit::ImageMemoryBarrier(image, oldLayout, newLayout);
		
		vk::PipelineStageFlags sourceStage{};
		vk::PipelineStageFlags destinationStage{};
		
		if (newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
		{
			barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
		
			if (HasStencilComponent(format))
			{
				barrier.subresourceRange.aspectMask |= vk::ImageAspectFlagBits::eStencil;
			}
		}
		else
		{
			barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		}
		
		if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal)
		{
			barrier.srcAccessMask = {};
			barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
		
			sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
			destinationStage = vk::PipelineStageFlagBits::eTransfer;
		}
		else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
		{
			barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
			barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
		
			sourceStage = vk::PipelineStageFlagBits::eTransfer;
			destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
		}
		else if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
		{
			barrier.srcAccessMask = {};
			barrier.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
		
			sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
			destinationStage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
		}
		else
		{
			throw std::runtime_error("Unsupported layout transition!");
		}

		cmd.pipelineBarrier(
			sourceStage,
			destinationStage,
			vk::DependencyFlags(),
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		VulkanHelpers::EndSingleTimeCommands(cmd);
	}
	
	void VulkanRenderer::CopyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height) const
	{
		const vk::CommandBuffer cmd = VulkanHelpers::BeginSingleTimeCommands();

		const vk::BufferImageCopy region = vk::BufferImageCopy()
			.setImageSubresource(vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1))
			.setImageExtent(vk::Extent3D(width, height, 1));

		cmd.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, region);

		VulkanHelpers::EndSingleTimeCommands(cmd);
	}
	
	vk::ImageView VulkanRenderer::CreateImageView(vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags) const
	{
		const vk::ImageViewCreateInfo viewInfo = vk::ImageViewCreateInfo()
			.setImage(image)
			.setViewType(vk::ImageViewType::e2D)
			.setFormat(format)
			.setSubresourceRange(vk::ImageSubresourceRange(aspectFlags, 0, 1, 0, 1));

		vk::ImageView imageView;
		try
		{
			imageView = m_pDevice->GetDevice().createImageView(viewInfo);
		}
		catch(vk::SystemError& e)
		{
			throw std::runtime_error("Failed to create image view: "s + e.what());
		}

		return imageView;
	}

	vk::Format VulkanRenderer::FindSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling,
		vk::FormatFeatureFlags features)
	{
		for (vk::Format format : candidates)
		{
			vk::FormatProperties props = m_pDevice->GetPhysicalDevice().getFormatProperties(format);

			if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features)
			{
				return format;
			}
			else if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features)
			{
				return format;
			}
		}

		throw std::runtime_error("Failed to find supported format!");
	}

	vk::Format VulkanRenderer::FindDepthFormat()
	{
		return FindSupportedFormat(
			{ vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint },
			vk::ImageTiling::eOptimal,
			vk::FormatFeatureFlagBits::eDepthStencilAttachment
		);
	}

	bool VulkanRenderer::HasStencilComponent(vk::Format format) const
	{
		return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint;
	}

	void VulkanRenderer::BeginCommandBuffers()
	{
		const vk::CommandBufferBeginInfo beginInfo = vk::CommandBufferBeginInfo();

		const vk::CommandBuffer& cmd = m_CommandBuffers[m_CurrentBuffer];

		try
		{
			cmd.begin(beginInfo);
		}
		catch (vk::SystemError& e)
		{
			throw std::runtime_error("Failed to begin command buffer: "s + e.what());
		}

		std::array<vk::ClearValue, 2> clearValues{};
		clearValues[0].setColor(vk::ClearColorValue(std::array<float, 4>{0.1f, 0.1f, 0.1f, 1.0f}));
		clearValues[1].setDepthStencil(vk::ClearDepthStencilValue(1.0f, 0));

		const vk::RenderPassBeginInfo renderPassInfo = vk::RenderPassBeginInfo()
			.setRenderPass(m_RenderPass)
			.setFramebuffer(m_pSwapChain->GetFramebuffers()[m_CurrentBuffer])
			.setRenderArea(vk::Rect2D()
				.setOffset(vk::Offset2D(0, 0))
				.setExtent(m_pSwapChain->GetExtent()))
			.setClearValues(clearValues);

		cmd.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
		cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, m_GraphicsPipeline);
	}

	void VulkanRenderer::EndCommandBuffers()
	{
		vk::CommandBuffer& cmd = m_CommandBuffers[m_CurrentBuffer];

		cmd.endRenderPass();

		try
		{
			cmd.end();
		}
		catch (vk::SystemError& e)
		{
			throw std::runtime_error("Failed to end command buffer: "s + e.what());
		}
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
