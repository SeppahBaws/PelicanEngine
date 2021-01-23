#pragma once

#include <vulkan/vulkan.h>

namespace Pelican
{
	class VulkanDevice;

	class VulkanSwapChain final
	{
	public:
		VulkanSwapChain(VulkanDevice* pDevice);
		~VulkanSwapChain();

		void Initialize();
		void CreateFramebuffers(VkImageView depthImageView, VkRenderPass renderPass);
		void Cleanup();

		VkSwapchainKHR GetSwapChain() const { return m_SwapChain; }
		VkFormat GetImageFormat() const { return m_SwapChainImageFormat; }
		VkExtent2D GetExtent() const { return m_SwapChainExtent; }
		std::vector<VkImageView> GetImageViews() const { return m_SwapChainImageViews; }
		std::vector<VkImage> GetImages() const { return m_SwapChainImages; }
		std::vector<VkFramebuffer> GetFramebuffers() const { return m_Framebuffers; }

	private:
		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

		void CreateSwapChain();
		void CreateImageViews();

		// TODO: SwapChain should use textures, which then hold CreateImageView, but for now we'll just duplicate this function.
		VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

	private:
		VulkanDevice* m_pDevice{};

		VkSwapchainKHR m_SwapChain{};
		std::vector<VkImage> m_SwapChainImages{};
		VkFormat m_SwapChainImageFormat{};
		VkExtent2D m_SwapChainExtent{};
		std::vector<VkImageView> m_SwapChainImageViews{};
		std::vector<VkFramebuffer> m_Framebuffers{};
	};
}
