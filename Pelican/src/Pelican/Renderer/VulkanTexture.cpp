#include "PelicanPCH.h"
#include "VulkanTexture.h"

#include <stb_image.h>
#include <glm/vec4.hpp>

#include "VulkanDebug.h"
#include "VulkanHelpers.h"
#include "VulkanRenderer.h"

namespace Pelican
{
	VulkanTexture::VulkanTexture(const std::string& path)
		: BaseAsset(path)
	{
		InitFromFile(path);
	}

	// VulkanTexture::VulkanTexture(const glm::vec4& color, int width, int height)
	// {
	// 	InitFromColor(color, width, height);
	// }

	VulkanTexture::~VulkanTexture()
	{
		VulkanRenderer::GetDevice().destroySampler(m_ImageSampler);
		VulkanRenderer::GetDevice().destroyImageView(m_ImageView);
		VulkanRenderer::GetDevice().destroyImage(m_Image);
		VulkanRenderer::GetDevice().freeMemory(m_ImageMemory);
	}

	void VulkanTexture::InitFromFile(const std::string& path)
	{
		m_AssetPath = path;

		int width, height, nrChannels;
		stbi_uc* pixels = stbi_load(path.c_str(), &width, &height, &nrChannels, STBI_rgb_alpha);

		if (!pixels)
		{
			ASSERT_MSG(false, "failed to load texture image!");
		}

		CreateTextureImage(pixels, width, height, STBI_rgb_alpha);
		CreateTextureImageView();
		CreateTextureSampler();

		stbi_image_free(pixels);
	}

	void VulkanTexture::InitFromColor(const glm::vec4& color, int width, int height)
	{
		std::vector<glm::vec4> pixels{ color };

		CreateTextureImage(pixels.data(), width, height, 4);
		CreateTextureImageView();
		CreateTextureSampler();
	}

	void VulkanTexture::InitFromData(void* data, int width, int height, int channels)
	{
		CreateTextureImage(data, width, height, channels);
		CreateTextureImageView();
		CreateTextureSampler();
	}

	vk::DescriptorImageInfo VulkanTexture::GetDescriptorImageInfo() const
	{
		vk::DescriptorImageInfo info(
			m_ImageSampler,
			m_ImageView,
			vk::ImageLayout::eShaderReadOnlyOptimal);

		return info;
	}

	void VulkanTexture::CreateTextureImage(void* pixelData, int width, int height, int channels)
	{
		const vk::DeviceSize size = width * height * channels;

		vk::Buffer stagingBuffer;
		vk::DeviceMemory stagingBufferMemory;

		VulkanHelpers::CreateBuffer(
			size,
			vk::BufferUsageFlagBits::eTransferSrc,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
			stagingBuffer, stagingBufferMemory);

		void* data = VulkanRenderer::GetDevice().mapMemory(stagingBufferMemory, 0, size);
			memcpy(data, pixelData, static_cast<size_t>(size));
		VulkanRenderer::GetDevice().unmapMemory(stagingBufferMemory);

		CreateImage(width, height, vk::Format::eR8G8B8A8Srgb, vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal,
			m_Image, m_ImageMemory);

		TransitionImageLayout(m_Image, vk::Format::eR8G8B8A8Srgb, vk::ImageLayout::eUndefined,
			vk::ImageLayout::eTransferDstOptimal);

		CopyBufferToImage(stagingBuffer, m_Image, static_cast<uint32_t>(width), static_cast<uint32_t>(height));

		TransitionImageLayout(m_Image, vk::Format::eR8G8B8A8Srgb, vk::ImageLayout::eTransferDstOptimal,
			vk::ImageLayout::eShaderReadOnlyOptimal);

		VulkanRenderer::GetDevice().destroyBuffer(stagingBuffer);
		VulkanRenderer::GetDevice().freeMemory(stagingBufferMemory);

		VkDebugMarker::SetImageName(VulkanRenderer::GetDevice(), m_Image, m_AssetPath.c_str());
	}

	void VulkanTexture::CreateTextureImageView()
	{
		m_ImageView = CreateImageView(m_Image, vk::Format::eR8G8B8A8Srgb, vk::ImageAspectFlagBits::eColor);
	}

	void VulkanTexture::CreateTextureSampler()
	{
		vk::SamplerCreateInfo samplerInfo;
		samplerInfo.magFilter = vk::Filter::eLinear;
		samplerInfo.minFilter = vk::Filter::eLinear;
		samplerInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
		samplerInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
		samplerInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = 16.0f;
		samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
		samplerInfo.unnormalizedCoordinates = false;
		samplerInfo.compareEnable = false;
		samplerInfo.compareOp = vk::CompareOp::eAlways;
		samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;

		try
		{
			m_ImageSampler = VulkanRenderer::GetDevice().createSampler(samplerInfo);
		}
		catch (vk::SystemError& e)
		{
			throw std::runtime_error("Failed to create image sampler: "s + e.what());
		}
	}

	void VulkanTexture::CreateImage(uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling,
		vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Image& image, vk::DeviceMemory& imageMemory)
	{
		vk::ImageCreateInfo imageInfo;
		imageInfo.imageType = vk::ImageType::e2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = vk::ImageLayout::eUndefined;
		imageInfo.usage = usage;
		imageInfo.sharingMode = vk::SharingMode::eExclusive;
		imageInfo.samples = vk::SampleCountFlagBits::e1;
		imageInfo.flags = {};

		try
		{
			image = VulkanRenderer::GetDevice().createImage(imageInfo);
		}
		catch (vk::SystemError& e)
		{
			throw std::runtime_error("Failed to create image: "s + e.what());
		}

		vk::MemoryRequirements memRequirements = VulkanRenderer::GetDevice().getImageMemoryRequirements(image);
		vk::MemoryAllocateInfo allocInfo(
			memRequirements.size,
			VulkanHelpers::FindMemoryType(memRequirements.memoryTypeBits, properties));

		try
		{
			imageMemory = VulkanRenderer::GetDevice().allocateMemory(allocInfo);
		}
		catch (vk::SystemError& e)
		{
			throw std::runtime_error("Failed to allocate image memory: "s + e.what());
		}

		VulkanRenderer::GetDevice().bindImageMemory(image, imageMemory, 0);
	}

	vk::ImageView VulkanTexture::CreateImageView(vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags)
	{
		vk::ImageViewCreateInfo viewInfo;
		viewInfo.image = image;
		viewInfo.viewType = vk::ImageViewType::e2D;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		vk::ImageView imageView;
		try
		{
			imageView = VulkanRenderer::GetDevice().createImageView(viewInfo);
		}
		catch (vk::SystemError& e)
		{
			throw std::runtime_error("Failed to create image view: "s + e.what());
		}

		return imageView;
	}

	void VulkanTexture::TransitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldLayout,
		vk::ImageLayout newLayout)
	{
		vk::CommandBuffer commandBuffer = VulkanHelpers::BeginSingleTimeCommands();

		vk::ImageMemoryBarrier barrier;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		vk::PipelineStageFlags sourceStage{};
		vk::PipelineStageFlags destinationStage{};

		if (newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
		{
			barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;

			// Check if the format has a stencil component.
			if (format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint)
			{
				barrier.subresourceRange.aspectMask |= vk::ImageAspectFlagBits::eStencil;
			}
		}
		else
		{
			barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		}

		if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal)
		{
			barrier.srcAccessMask = {};
			barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

			sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
			destinationStage = vk::PipelineStageFlagBits::eTransfer;
		}
		else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
		{
			barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
			barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

			sourceStage = vk::PipelineStageFlagBits::eTransfer;
			destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
		}
		else if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
		{
			barrier.srcAccessMask = {};
			barrier.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;

			sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
			destinationStage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
		}
		else
		{
			throw std::runtime_error("Unsupported layout transition!");
		}

		commandBuffer.pipelineBarrier(sourceStage, destinationStage, {}, {}, {}, barrier);

		VulkanHelpers::EndSingleTimeCommands(commandBuffer);
	}

	void VulkanTexture::CopyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height)
	{
		vk::CommandBuffer commandBuffer = VulkanHelpers::BeginSingleTimeCommands();

		vk::BufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;
		region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;
		region.imageOffset = vk::Offset3D(0, 0, 0);
		region.imageExtent = vk::Extent3D(width, height, 1);

		commandBuffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, region);

		VulkanHelpers::EndSingleTimeCommands(commandBuffer);
	}
}
