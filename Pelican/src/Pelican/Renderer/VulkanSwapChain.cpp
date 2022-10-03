#include "PelicanPCH.h"
#include "VulkanSwapChain.h"

#include "VkInit.h"
#include "VulkanDevice.h"
#include "VulkanHelpers.h"
#include "VulkanDebug.h"
#include "Pelican/Core/Application.h"
#include "Pelican/Core/Window.h"

namespace Pelican
{
	VulkanSwapChain::VulkanSwapChain(Context* pContext, VulkanDevice* pDevice)
		: m_pContext(pContext)
		, m_pDevice(pDevice)
	{
		Initialize();
	}

	VulkanSwapChain::~VulkanSwapChain()
	{
		Cleanup();
	}

	void VulkanSwapChain::Initialize()
	{
		CreateSwapChain();
		CreateImageViews();
	}

	void VulkanSwapChain::CreateFramebuffers(vk::ImageView depthImageView, vk::RenderPass renderPass)
	{
		m_Framebuffers.resize(m_SwapChainImageViews.size());

		for (size_t i = 0; i < m_SwapChainImageViews.size(); i++)
		{
			std::array<vk::ImageView, 2> attachments = {
				m_SwapChainImageViews[i],
				depthImageView
			};

			vk::FramebufferCreateInfo framebufferInfo(
				{},
				renderPass,
				static_cast<uint32_t>(attachments.size()),
				attachments.data(),
				m_SwapChainExtent.width,
				m_SwapChainExtent.height,
				1
			);

			try
			{
				m_Framebuffers[i] = m_pDevice->GetDevice().createFramebuffer(framebufferInfo);
			}
			catch (vk::SystemError& e)
			{
				throw std::runtime_error("Failed to create framebuffer: "s + e.what());
			}

			VkDebugMarker::SetFramebufferName(m_pDevice->GetDevice(), m_Framebuffers[i], std::string("Framebuffer " + i).c_str());
		}
	}

	void VulkanSwapChain::Cleanup()
	{
		// Due to RAII, when closing the window this function gets called twice
		// hence why we check if the SwapChain has been cleared already.
		if (!m_SwapChain)
			return;

		for (vk::Framebuffer framebuffer : m_Framebuffers)
		{
			m_pDevice->GetDevice().destroyFramebuffer(framebuffer);
		}

		for (size_t i = 0; i < m_SwapChainImageViews.size(); i++)
		{
			m_pDevice->GetDevice().destroyImageView(m_SwapChainImageViews[i]);
			m_SwapChainImageViews[i] = nullptr;
		}

		m_pDevice->GetDevice().destroySwapchainKHR(m_SwapChain);
		m_SwapChain = nullptr;
	}

	vk::SurfaceFormatKHR VulkanSwapChain::ChooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats) const
	{
		for (const auto& availableFormat : availableFormats)
		{
			if (availableFormat.format == vk::Format::eB8G8R8A8Unorm && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
			{
				return availableFormat;
			}
		}

		return availableFormats[0];
	}

	vk::Extent2D VulkanSwapChain::ChooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities) const
	{
		if (capabilities.currentExtent.width != UINT32_MAX)
		{
			return capabilities.currentExtent;
		}

		const WindowSpecification& windowSpec = m_pContext->GetSubsystem<Window>()->GetSpecification();

		vk::Extent2D actualExtent = { static_cast<uint32_t>(windowSpec.width), static_cast<uint32_t>(windowSpec.height) };

		actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}

	void VulkanSwapChain::CreateSwapChain()
	{
		SwapChainSupportDetails swapChainSupport = VulkanHelpers::QuerySwapChainSupport(m_pDevice->GetPhysicalDevice(), m_pDevice->GetSurface());

		vk::SurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
		vk::Extent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

		if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
		{
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}

		vk::SwapchainCreateInfoKHR createInfo;
		createInfo.surface = m_pDevice->GetSurface();
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

		QueueFamilyIndices indices = m_pDevice->FindQueueFamilies();
		uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		if (indices.graphicsFamily != indices.presentFamily)
		{
			createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else
		{
			createInfo.imageSharingMode = vk::SharingMode::eExclusive;
			createInfo.queueFamilyIndexCount = 0;
			createInfo.pQueueFamilyIndices = nullptr;
		}

		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
		createInfo.presentMode = vk::PresentModeKHR::eFifo; // TODO: add toggle for vsync.
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = nullptr;

		try
		{
			m_SwapChain = m_pDevice->GetDevice().createSwapchainKHR(createInfo);
		}
		catch (vk::SystemError& e)
		{
			throw std::runtime_error("Failed to create the swapchain: "s + e.what());
		}

		vk::Result result = m_pDevice->GetDevice().getSwapchainImagesKHR(m_SwapChain, &imageCount, nullptr);
		if (result != vk::Result::eSuccess)
		{
			throw std::runtime_error("Failed to get the swapchain images");
		}

		m_SwapChainImages.resize(imageCount);

		result = m_pDevice->GetDevice().getSwapchainImagesKHR(m_SwapChain, &imageCount, m_SwapChainImages.data());
		if (result != vk::Result::eSuccess)
		{
			throw std::runtime_error("Failed to get the swapchain images");
		}

		m_SwapChainImageFormat = surfaceFormat.format;
		m_SwapChainExtent = extent;
	}

	void VulkanSwapChain::CreateImageViews()
	{
		m_SwapChainImageViews.resize(m_SwapChainImages.size());

		for (size_t i = 0; i < m_SwapChainImages.size(); i++)
		{
			m_SwapChainImageViews[i] = CreateImageView(m_SwapChainImages[i], m_SwapChainImageFormat, vk::ImageAspectFlagBits::eColor);
		}
	}

	vk::ImageView VulkanSwapChain::CreateImageView(vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags) const
	{
		const vk::ImageViewCreateInfo viewInfo(
			{},
			image,
			vk::ImageViewType::e2D,
			format,
			{},
			vk::ImageSubresourceRange{ aspectFlags, 0, 1, 0, 1 }
		);

		vk::ImageView imageView;

		try
		{
			imageView = m_pDevice->GetDevice().createImageView(viewInfo);
		}
		catch (vk::SystemError& e)
		{
			throw std::runtime_error("Failed to create image view: "s + e.what());
		}

		return imageView;
	}
}
