#pragma once
#include <vulkan/vulkan.hpp>

namespace Pelican
{
	// class VulkanImage
	// {
	// public:
	// 	VulkanImage(u32 width, u32 height);
	//
	// 	void TransitionLayout(vk::Format newFormat, vk::ImageLayout newLayout);
	//
	// 	void Bind(vk::DeviceSize memoryOffset);
	//
	// private:
	// 	void CreateImage(vk::Format format, vk::ImageTiling tiling,
	// 	                 vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties);
	// 	void CreateImageView(vk::Format format, vk::ImageAspectFlags aspectFlags);
	//
	// private:
	// 	u32 m_Width, m_Height;
	//
	// 	vk::Image m_Image;
	// 	vk::DeviceMemory m_Memory;
	// 	vk::ImageView m_ImageView;
	//
	// 	vk::ImageLayout m_Layout;
	// 	vk::Format m_Format;
	// };
}
