#pragma once
#include <vulkan/vulkan.hpp>

#include <glm/vec4.hpp>

#include "Pelican/Assets/BaseAsset.h"

namespace Pelican
{
	class VulkanTexture : public BaseAsset
	{
	public:
		enum class TextureMode
		{
			Texture2d,
			Cubemap,
		};

		// VulkanTexture();
		explicit VulkanTexture(const std::string& path, TextureMode mode);
		// TODO: implement this
		// explicit VulkanTexture(const glm::vec4& color, int width, int height);
		~VulkanTexture();

		void InitFromFile(const std::string& path);

		// Ignore these, they don't work atm.
		void InitFromColor(const glm::vec4& color, int width, int height);
		void InitFromData(void* data, int width, int height, int channels);

		void TransitionLayout(vk::ImageLayout newLayout);

		[[nodiscard]] vk::ImageView GetImageView() const { return m_ImageView; }
		[[nodiscard]] vk::Sampler GetSampler() const { return m_ImageSampler; }

		[[nodiscard]] vk::DescriptorImageInfo GetDescriptorImageInfo() const;

	private:
		void CreateTextureImage(void* pixelData, int width, int height, int channels);
		void CreateTextureImageView();
		void CreateTextureSampler();

	private:
		void CreateImage(uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling,
			vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties);
		vk::ImageView CreateImageView(vk::ImageAspectFlags aspectFlags);
		void CopyBufferToImage(vk::Buffer buffer);
		void CopyBufferToImageCubemap(vk::Buffer buffer);

	private:
		vk::Image m_Image{};
		vk::DeviceMemory m_ImageMemory{};
		vk::ImageView m_ImageView{};
		vk::Sampler m_ImageSampler{};

		TextureMode m_TextureMode{};
		uint32_t m_Width{};
		uint32_t m_Height{};
		uint32_t m_LayerCount{};
		vk::Format m_Format{};
		vk::ImageLayout m_ImageLayout{};
	};
}
