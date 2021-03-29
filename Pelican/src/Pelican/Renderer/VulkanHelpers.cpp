#include "PelicanPCH.h"
#include "VulkanHelpers.h"

#include <string>

#include "VkInit.h"
#include "VulkanRenderer.h"

namespace Pelican
{
	vk::CommandBuffer VulkanHelpers::BeginSingleTimeCommands()
	{
		const vk::CommandBufferAllocateInfo allocInfo = vk::CommandBufferAllocateInfo()
			.setCommandPool(VulkanRenderer::GetCommandPool())
			.setCommandBufferCount(1)
			.setLevel(vk::CommandBufferLevel::ePrimary);

		const vk::CommandBuffer cmd = VulkanRenderer::GetDevice().allocateCommandBuffers(allocInfo)[0];

		const vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
		cmd.begin(beginInfo);

		return cmd;
	}

	void VulkanHelpers::EndSingleTimeCommands(vk::CommandBuffer commandBuffer)
	{
		commandBuffer.end();

		vk::SubmitInfo submitInfo{};
		submitInfo.setCommandBuffers(commandBuffer);

		VulkanRenderer::GetGraphicsQueue().submit(submitInfo);
		VulkanRenderer::GetGraphicsQueue().waitIdle();

		VulkanRenderer::GetDevice().freeCommandBuffers(VulkanRenderer::GetCommandPool(), commandBuffer);
	}

	uint32_t VulkanHelpers::FindMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties)
	{
		vk::PhysicalDeviceMemoryProperties memProperties = VulkanRenderer::GetPhysicalDevice().getMemoryProperties();

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
		{
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}

		throw std::runtime_error("Failed to find suitable memory type!");
	}

	void VulkanHelpers::CreateBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties,
	                                 vk::Buffer& buffer, vk::DeviceMemory& bufferMemory)
	{
		vk::BufferCreateInfo bufferInfo({}, size, usage, vk::SharingMode::eExclusive);

		try
		{
			buffer = VulkanRenderer::GetDevice().createBuffer(bufferInfo);
		}
		catch (vk::SystemError& e)
		{
			throw std::runtime_error("Failed to create buffer: "s + e.what());
		}

		vk::MemoryRequirements memRequirements = VulkanRenderer::GetDevice().getBufferMemoryRequirements(buffer);

		vk::MemoryAllocateInfo allocInfo(
			memRequirements.size,
			FindMemoryType(memRequirements.memoryTypeBits, properties)
		);

		try
		{
			bufferMemory = VulkanRenderer::GetDevice().allocateMemory(allocInfo);
		}
		catch (vk::SystemError& e)
		{
			throw std::runtime_error("Failed to allocate buffer memory: "s + e.what());
		}

		try
		{
			VulkanRenderer::GetDevice().bindBufferMemory(buffer, bufferMemory, 0);
		}
		catch (vk::SystemError& e)
		{
			throw std::runtime_error("Failed to bind buffer memory: "s + e.what());
		}
	}

	void VulkanHelpers::CopyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size)
	{
		vk::CommandBuffer commandBuffer = BeginSingleTimeCommands();

		vk::BufferCopy copyRegion({}, {}, size);
		commandBuffer.copyBuffer(srcBuffer, dstBuffer, 1, &copyRegion);

		EndSingleTimeCommands(commandBuffer);
	}

	bool VulkanHelpers::CheckDeviceExtensionSupport(vk::PhysicalDevice physicalDevice)
	{
		using namespace std::string_literals;

		std::vector<vk::ExtensionProperties> availableExtensions = physicalDevice.enumerateDeviceExtensionProperties(""s);
		std::set<std::string> requiredExtensions(g_DeviceExtensions.begin(), g_DeviceExtensions.end());

		for (const auto& extension : availableExtensions)
		{
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}

	SwapChainSupportDetails VulkanHelpers::QuerySwapChainSupport(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface)
	{
		SwapChainSupportDetails details;

		details.capabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface);
		details.formats = physicalDevice.getSurfaceFormatsKHR(surface);
		details.presentModes = physicalDevice.getSurfacePresentModesKHR(surface);

		return details;
	}
}
