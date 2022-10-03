#include "PelicanPCH.h"
#include "VulkanImage.h"

#include "VkInit.h"
#include "VulkanRenderer.h"
#include "VulkanHelpers.h"

namespace Pelican
{
	// VulkanImage::VulkanImage(u32 width, u32 height)
	// 	: m_Width(width), m_Height(height)
	// {
	// 	// CreateImage()
	// }
	//
	// void VulkanImage::TransitionLayout(vk::Format newFormat, vk::ImageLayout newLayout)
	// {
	// 	const vk::CommandBuffer cmd = VulkanHelpers::BeginSingleTimeCommands();
	//
	// 	vk::ImageMemoryBarrier barrier = VkInit::ImageMemoryBarrier(m_Image, m_Layout, newLayout);
	//
	// 	vk::PipelineStageFlags sourceStage{};
	// 	vk::PipelineStageFlags destStage{};
	// }
	//
	// void VulkanImage::Bind(vk::DeviceSize memoryOffset)
	// {
	// 	VulkanRenderer::GetDevice().bindImageMemory(m_Image, m_Memory, memoryOffset);
	// }
	//
	// void VulkanImage::CreateImage(vk::Format format, vk::ImageTiling tiling,
	//                               vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties)
	// {
	// 	vk::ImageCreateInfo imageInfo{};
	// 	imageInfo.imageType = vk::ImageType::e2D;
	// 	imageInfo.extent = vk::Extent3D(m_Width, m_Height, 1);
	// 	imageInfo.format = format;
	// 	imageInfo.tiling = tiling;
	// 	imageInfo.usage = usage;
	// 	imageInfo.mipLevels = 1;
	// 	imageInfo.arrayLayers = 1;
	// 	imageInfo.initialLayout = vk::ImageLayout::eUndefined;
	// 	imageInfo.sharingMode = vk::SharingMode::eExclusive;
	// 	imageInfo.samples = vk::SampleCountFlagBits::e1;
	//
	// 	try
	// 	{
	// 		m_Image = VulkanRenderer::GetDevice().createImage(imageInfo);
	// 	}
	// 	catch (vk::SystemError& e)
	// 	{
	// 		throw std::runtime_error("Failed to create image: "s + e.what());
	// 	}
	//
	// 	const vk::MemoryRequirements memRequirements = VulkanRenderer::GetDevice().getImageMemoryRequirements(m_Image);
	//
	// 	vk::MemoryAllocateInfo allocInfo{};
	// 	allocInfo.allocationSize = memRequirements.size;
	// 	allocInfo.memoryTypeIndex = VulkanHelpers::FindMemoryType(memRequirements.memoryTypeBits, properties);
	//
	// 	m_Memory = VulkanRenderer::GetDevice().allocateMemory(allocInfo);
	// }
	//
	// void VulkanImage::CreateImageView(vk::Format format, vk::ImageAspectFlags aspectFlags)
	// {
	// 	vk::ImageViewCreateInfo viewInfo{};
	// 	viewInfo.image = m_Image;
	// 	viewInfo.viewType = vk::ImageViewType::e2D;
	// 	viewInfo.format = format;
	// 	viewInfo.subresourceRange = vk::ImageSubresourceRange(aspectFlags, 0, 1, 0, 1);
	//
	// 	try
	// 	{
	// 		m_ImageView = VulkanRenderer::GetDevice().createImageView(viewInfo);
	// 	}
	// 	catch (vk::SystemError& e)
	// 	{
	// 		throw std::runtime_error("Failed to create image view: "s + e.what());
	// 	}
	// }
}
