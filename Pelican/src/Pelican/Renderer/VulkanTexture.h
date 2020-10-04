#pragma once
#include <vulkan/vulkan.h>

namespace Pelican
{
	class VulkanTexture
	{
	public:
		VulkanTexture(const std::string& path);
		~VulkanTexture();

		VkImageView GetImageView() const { return m_ImageView; }
		VkSampler GetSampler() const { return m_ImageSampler; }

	private:
		void Init();
		void Cleanup();

		void CreateTextureImage();
		void CreateTextureImageView();
		void CreateTextureSampler();

	private:
		void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
			VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
		VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
		void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
		void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

	private:
		VkImage m_Image;
		VkDeviceMemory m_ImageMemory;
		VkImageView m_ImageView;
		VkSampler m_ImageSampler;

		std::string m_TexturePath;
	};
}
