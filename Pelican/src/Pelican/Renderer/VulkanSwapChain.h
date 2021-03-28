#pragma once

#include <vulkan/vulkan.hpp>

namespace Pelican
{
	class VulkanDevice;

	class VulkanSwapChain final
	{
	public:
		VulkanSwapChain(VulkanDevice* pDevice);
		~VulkanSwapChain();

		void Initialize();
		void CreateFramebuffers(vk::ImageView depthImageView, vk::RenderPass renderPass);
		void Cleanup();

		vk::SwapchainKHR GetSwapChain() const { return m_SwapChain; }
		vk::Format GetImageFormat() const { return m_SwapChainImageFormat; }
		vk::Extent2D GetExtent() const { return m_SwapChainExtent; }
		std::vector<vk::ImageView> GetImageViews() const { return m_SwapChainImageViews; }
		std::vector<vk::Image> GetImages() const { return m_SwapChainImages; }
		std::vector<vk::Framebuffer> GetFramebuffers() const { return m_Framebuffers; }

	private:
		vk::SurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats) const;
		vk::PresentModeKHR ChooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes) const;
		vk::Extent2D ChooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities) const;

		void CreateSwapChain();
		void CreateImageViews();

		// TODO: SwapChain should use textures, which then hold CreateImageView, but for now we'll just duplicate this function.
		vk::ImageView CreateImageView(vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags) const;

	private:
		VulkanDevice* m_pDevice{};

		vk::SwapchainKHR m_SwapChain{};
		std::vector<vk::Image> m_SwapChainImages{};
		vk::Format m_SwapChainImageFormat{};
		vk::Extent2D m_SwapChainExtent{};
		std::vector<vk::ImageView> m_SwapChainImageViews{};
		std::vector<vk::Framebuffer> m_Framebuffers{};
	};
}
