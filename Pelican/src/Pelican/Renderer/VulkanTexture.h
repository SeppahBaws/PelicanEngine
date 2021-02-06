#pragma once
#include <vulkan/vulkan.h>

#include <glm/vec4.hpp>

namespace Pelican
{
	class VulkanTexture
	{
	public:
		// VulkanTexture();
		explicit VulkanTexture(const std::string& path);
		// TODO: implement this
		// explicit VulkanTexture(const glm::vec4& color, int width, int height);
		~VulkanTexture();

		void InitFromFile(const std::string& path);
		void InitFromColor(const glm::vec4& color, int width, int height);

		[[nodiscard]] VkImageView GetImageView() const { return m_ImageView; }
		[[nodiscard]] VkSampler GetSampler() const { return m_ImageSampler; }

		[[nodiscard]] VkDescriptorImageInfo GetDescriptorImageInfo() const;

	private:
		void CreateTextureImage(void* pixelData, int width, int height, int channels);
		void CreateTextureImageView();
		void CreateTextureSampler();

	private:
		void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
			VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
		VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
		void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
		void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

	private:
		VkImage m_Image{};
		VkDeviceMemory m_ImageMemory{};
		VkImageView m_ImageView{};
		VkSampler m_ImageSampler{};

		std::string m_TexturePath{};
	};
}
