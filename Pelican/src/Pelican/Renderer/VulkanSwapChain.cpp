﻿#include "PelicanPCH.h"
#include "VulkanSwapChain.h"

#include "VkInit.h"
#include "VulkanDevice.h"
#include "VulkanHelpers.h"
#include "Pelican/Core/Application.h"
#include "Pelican/Core/Window.h"

namespace Pelican
{
	VulkanSwapChain::VulkanSwapChain(VulkanDevice* pDevice)
		: m_pDevice(pDevice)
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

	void VulkanSwapChain::CreateFramebuffers(VkImageView depthImageView, VkRenderPass renderPass)
	{
		m_Framebuffers.resize(m_SwapChainImageViews.size());

		for (size_t i = 0; i < m_SwapChainImageViews.size(); i++)
		{
			std::array<VkImageView, 2> attachments = {
				m_SwapChainImageViews[i],
				depthImageView
			};

			VkFramebufferCreateInfo framebufferInfo = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
			framebufferInfo.renderPass = renderPass;
			framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = m_SwapChainExtent.width;
			framebufferInfo.height = m_SwapChainExtent.height;
			framebufferInfo.layers = 1;

			VK_CHECK(vkCreateFramebuffer(m_pDevice->GetDevice(), &framebufferInfo, nullptr, &m_Framebuffers[i]));
		}
	}

	void VulkanSwapChain::Cleanup()
	{
		// Due to RAII, when closing the window this function gets called twice
		// hence why we check if the SwapChain has been cleared already.
		if (m_SwapChain == VK_NULL_HANDLE)
			return;

		for (VkFramebuffer framebuffer : m_Framebuffers)
		{
			vkDestroyFramebuffer(m_pDevice->GetDevice(), framebuffer, nullptr);
		}

		for (size_t i = 0; i < m_SwapChainImageViews.size(); i++)
		{
			vkDestroyImageView(m_pDevice->GetDevice(), m_SwapChainImageViews[i], nullptr);
			m_SwapChainImageViews[i] = VK_NULL_HANDLE;
		}

		vkDestroySwapchainKHR(m_pDevice->GetDevice(), m_SwapChain, nullptr);
		m_SwapChain = VK_NULL_HANDLE;
	}

	VkSurfaceFormatKHR VulkanSwapChain::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
	{
		for (const auto& availableFormat : availableFormats)
		{
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR)
			{
				return availableFormat;
			}
		}

		return availableFormats[0];
	}

	VkPresentModeKHR VulkanSwapChain::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
	{
		// TODO: return FIFO if we want VSync, otherwise return the next best thing.

		for (const auto& availablePresentMode : availablePresentModes)
		{
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				return availablePresentMode;
			}
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D VulkanSwapChain::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
	{
		if (capabilities.currentExtent.width != UINT32_MAX)
		{
			return capabilities.currentExtent;
		}

		Window::Params params = Application::Get().GetWindow()->GetParams();

		VkExtent2D actualExtent = { static_cast<uint32_t>(params.width), static_cast<uint32_t>(params.height) };

		actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}

	void VulkanSwapChain::CreateSwapChain()
	{
		SwapChainSupportDetails swapChainSupport = VulkanHelpers::QuerySwapChainSupport(m_pDevice->GetPhysicalDevice(), m_pDevice->GetSurface());

		VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
		VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

		if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
		{
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR createInfo = VkInit::SwapchainCreateInfo();
		createInfo.surface = m_pDevice->GetSurface();
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		QueueFamilyIndices indices = m_pDevice->FindQueueFamilies();
		uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		if (indices.graphicsFamily != indices.presentFamily)
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0;
			createInfo.pQueueFamilyIndices = nullptr;
		}

		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		VK_CHECK(vkCreateSwapchainKHR(m_pDevice->GetDevice(), &createInfo, nullptr, &m_SwapChain));

		vkGetSwapchainImagesKHR(m_pDevice->GetDevice(), m_SwapChain, &imageCount, nullptr);
		m_SwapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(m_pDevice->GetDevice(), m_SwapChain, &imageCount, m_SwapChainImages.data());
		m_SwapChainImageFormat = surfaceFormat.format;
		m_SwapChainExtent = extent;
	}

	void VulkanSwapChain::CreateImageViews()
	{
		m_SwapChainImageViews.resize(m_SwapChainImages.size());

		for (size_t i = 0; i < m_SwapChainImages.size(); i++)
		{
			m_SwapChainImageViews[i] = CreateImageView(m_SwapChainImages[i], m_SwapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
		}
	}

	VkImageView VulkanSwapChain::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
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
}
