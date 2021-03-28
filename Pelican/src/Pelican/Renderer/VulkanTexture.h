#pragma once
#include <vulkan/vulkan.hpp>

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

		[[nodiscard]] vk::ImageView GetImageView() const { return m_ImageView; }
		[[nodiscard]] vk::Sampler GetSampler() const { return m_ImageSampler; }

		[[nodiscard]] vk::DescriptorImageInfo GetDescriptorImageInfo() const;

	private:
		void CreateTextureImage(void* pixelData, int width, int height, int channels);
		void CreateTextureImageView();
		void CreateTextureSampler();

	private:
		void CreateImage(uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling,
			vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Image& image, vk::DeviceMemory& imageMemory);
		vk::ImageView CreateImageView(vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags);
		void TransitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout);
		void CopyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height);

	private:
		vk::Image m_Image{};
		vk::DeviceMemory m_ImageMemory{};
		vk::ImageView m_ImageView{};
		vk::Sampler m_ImageSampler{};

		std::string m_TexturePath{};
	};
}
