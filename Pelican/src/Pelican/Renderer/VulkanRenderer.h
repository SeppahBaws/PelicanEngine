#pragma once

#include <vulkan/vulkan.hpp>

#include "VulkanDevice.h"
#include "VulkanPipeline.h"
#include "VulkanSwapChain.h"
#include "Pelican/Core/Subsystem.h"

namespace Pelican
{
	class VulkanBuffer;
	class Model;
	class VulkanTexture;
	// Forward declarations
	class Camera;
	class ImGuiWrapper;

	enum class RenderMode : int
	{
		Filled = 0,
		Lines,
		Points,

		// Keep as last!!!
		RENDERING_MODE_MAX
	};

	class VulkanRenderer final : public Subsystem
	{
	public:
		VulkanRenderer(Context* pContext);

		bool OnInitialize() override;
		void OnShutdown() override;

		// Very important: returns true when the scene is ready to be rendered.
		// For example: when the window gets resized, we want to skip this frame, because we'll be recreating the command buffers
		// so we won't be able to record to them.
		bool BeginScene();
		void EndScene();

		void WaitForIdle();

		void FlagWindowResized() { m_FrameBufferResized = true; }
		void SetCamera(Camera* pCamera);

		void ReloadShaders();

	public:
		static int GetMaxImages() { return m_pInstance->MAX_FRAMES_IN_FLIGHT; }
		static vk::Instance GetInstance() { return m_pInstance->m_Instance.get(); }
		static VulkanDevice* GetVulkanDevice() { return m_pInstance->m_pDevice; }
		static VulkanSwapChain* GetSwapChain() { return m_pInstance->m_pSwapChain; }
		static vk::Device GetDevice() { return m_pInstance->m_pDevice->GetDevice(); }
		static vk::RenderPass GetRenderPass() { return m_pInstance->m_RenderPass; }
		static vk::PhysicalDevice GetPhysicalDevice() { return m_pInstance->m_pDevice->GetPhysicalDevice(); }
		static vk::Queue GetGraphicsQueue() { return m_pInstance->m_pDevice->GetGraphicsQueue(); }
		static vk::DescriptorPool GetDescriptorPool() { return m_pInstance->m_DescriptorPool; }
		static vk::DescriptorSetLayout& GetDescriptorSetLayout() { return m_pInstance->m_DescriptorSetLayout; }
		static vk::CommandPool GetCommandPool() { return m_pInstance->m_CommandPool; }
		static vk::CommandBuffer GetCurrentBuffer() { return m_pInstance->m_CommandBuffers[m_pInstance->m_CurrentBuffer]; }
		static vk::Framebuffer GetCurrentFramebuffer() { return m_pInstance->m_pSwapChain->GetFramebuffers()[m_pInstance->m_CurrentBuffer]; }
		static vk::PipelineLayout GetPipelineLayout();
		static vk::Pipeline GetCurrentPipeline();
		static vk::PipelineLayout GetUnlitPipelineLayout() { return m_pInstance->m_UnlitPipeline.GetLayout(); }

#if TEST_ENABLE_SKYBOX
		static VulkanTexture* GetSkybox() { return m_pInstance->m_pSkyboxCubemap; }
#endif

	private:
		void CreateInstance();
		bool CheckValidationLayerSupport() const;
		std::vector<const char*> GetRequiredExtensions() const;
		void PrintExtensions() const;

		void CreateRenderPass();

		void CreateDescriptorSetLayout();
		void CreateGraphicsPipeline();

		void CreateCommandPool();
		void CreateDepthResources();
		void CreateUniformBuffers();
		void CreateDescriptorPool();
		void CreateCommandBuffers();

		void CreateSyncObjects();

		void CleanupSwapChain();
		void RecreateSwapChain();

		void ReloadShaders_Internal();

		void UpdateUniformBuffer(uint32_t currentImage);

		void CreateImage(uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling,
			vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Image& image, vk::DeviceMemory& imageMemory) const;
		void TransitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout) const;
		void CopyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height) const;
		vk::ImageView CreateImageView(vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags) const;

		vk::Format FindSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features);
		vk::Format FindDepthFormat();
		bool HasStencilComponent(vk::Format format) const;

		void BeginCommandBuffers();
		void EndCommandBuffers();

	private:
		Camera* m_pCamera = nullptr;

	private:
		static VulkanRenderer* m_pInstance;

		vk::UniqueInstance m_Instance;

		const bool m_EnableValidationLayers = PELICAN_VALIDATE;

		const std::vector<const char*> m_ValidationLayers = {
			"VK_LAYER_KHRONOS_validation"
		};

		VulkanDevice* m_pDevice{};
		VulkanSwapChain* m_pSwapChain{};

		vk::RenderPass m_RenderPass;
		vk::DescriptorSetLayout m_DescriptorSetLayout;

		VulkanPipeline m_Pipelines[static_cast<int>(RenderMode::RENDERING_MODE_MAX)];
		VulkanPipeline m_UnlitPipeline;

		vk::CommandPool m_CommandPool;
		std::vector<vk::CommandBuffer> m_CommandBuffers;
		const int MAX_FRAMES_IN_FLIGHT = 2;
		size_t m_CurrentFrame = 0;
		uint32_t m_CurrentBuffer;
		std::vector<vk::Semaphore> m_ImageAvailableSemaphores;
		std::vector<vk::Semaphore> m_RenderFinishedSemaphores;
		std::vector<vk::Fence> m_InFlightFences;
		std::vector<vk::Fence> m_ImagesInFlight;

		bool m_FrameBufferResized = false;

		bool m_ReloadShadersFlag = false;

		std::vector<VulkanBuffer*> m_MvpUBOs;
		std::vector<VulkanBuffer*> m_LightUBOs;
		vk::DescriptorPool m_DescriptorPool;

		vk::Image m_DepthImage;
		vk::DeviceMemory m_DepthImageMemory;
		vk::ImageView m_DepthImageView;



		// ImGui
		ImGuiWrapper* m_pImGui{};
	};
}
