#include "PelicanPCH.h"
#include "VulkanTexture.h"


#include <logtools.h>
#include <stb_image.h>
#include <glm/vec4.hpp>

#include "VulkanDebug.h"
#include "VulkanHelpers.h"
#include "VulkanRenderer.h"

namespace Pelican
{
	VulkanTexture::VulkanTexture(const std::string& path, TextureMode textureMode)
		: BaseAsset(path), m_TextureMode(textureMode)
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
			Logger::LogDebug("Tried loading texture \"%s\"", path.c_str());
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

	void VulkanTexture::TransitionLayout(vk::ImageLayout newLayout)
	{
		vk::CommandBuffer commandBuffer = VulkanHelpers::BeginSingleTimeCommands();

		vk::ImageMemoryBarrier barrier;
		barrier.oldLayout = m_ImageLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = m_Image;
		barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = m_LayerCount;

		vk::PipelineStageFlags sourceStage{};
		vk::PipelineStageFlags destinationStage{};

		if (newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
		{
			barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;

			// Check if the format has a stencil component.
			if (m_Format == vk::Format::eD32SfloatS8Uint || m_Format == vk::Format::eD24UnormS8Uint)
			{
				barrier.subresourceRange.aspectMask |= vk::ImageAspectFlagBits::eStencil;
			}
		}
		else
		{
			barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		}

		if (m_ImageLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal)
		{
			barrier.srcAccessMask = {};
			barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

			sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
			destinationStage = vk::PipelineStageFlagBits::eTransfer;
		}
		else if (m_ImageLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
		{
			barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
			barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

			sourceStage = vk::PipelineStageFlagBits::eTransfer;
			destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
		}
		else if (m_ImageLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
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

		m_ImageLayout = newLayout;
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
			vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal);

		TransitionLayout(vk::ImageLayout::eTransferDstOptimal);

		if (m_TextureMode == TextureMode::Cubemap)
		{
			CopyBufferToImageCubemap(stagingBuffer);
		}
		else
		{
			CopyBufferToImage(stagingBuffer);
		}

		TransitionLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

		VulkanRenderer::GetDevice().destroyBuffer(stagingBuffer);
		VulkanRenderer::GetDevice().freeMemory(stagingBufferMemory);

		VkDebugMarker::SetImageName(VulkanRenderer::GetDevice(), m_Image, m_AssetPath.c_str());
	}

	void VulkanTexture::CreateTextureImageView()
	{
		m_ImageView = CreateImageView(vk::ImageAspectFlagBits::eColor);
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
		vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties)
	{
		m_Width = width;
		m_Height = m_TextureMode == TextureMode::Cubemap ? width : height;
		m_Format = format;
		m_LayerCount = m_TextureMode == TextureMode::Cubemap ? 6 : 1;

		vk::ImageCreateFlags flags = m_TextureMode == TextureMode::Cubemap
			? vk::ImageCreateFlagBits::eCubeCompatible
			: vk::ImageCreateFlags{};

		vk::ImageCreateInfo imageInfo;
		imageInfo.imageType = vk::ImageType::e2D;
		imageInfo.extent.width = m_Width;
		imageInfo.extent.height = m_Height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = m_LayerCount;
		imageInfo.format = m_Format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = vk::ImageLayout::eUndefined;
		imageInfo.usage = usage;
		imageInfo.sharingMode = vk::SharingMode::eExclusive;
		imageInfo.samples = vk::SampleCountFlagBits::e1;
		imageInfo.flags = flags;

		try
		{
			m_Image = VulkanRenderer::GetDevice().createImage(imageInfo);
		}
		catch (vk::SystemError& e)
		{
			throw std::runtime_error("Failed to create image: "s + e.what());
		}

		vk::MemoryRequirements memRequirements = VulkanRenderer::GetDevice().getImageMemoryRequirements(m_Image);
		vk::MemoryAllocateInfo allocInfo(
			memRequirements.size,
			VulkanHelpers::FindMemoryType(memRequirements.memoryTypeBits, properties));

		try
		{
			m_ImageMemory = VulkanRenderer::GetDevice().allocateMemory(allocInfo);
		}
		catch (vk::SystemError& e)
		{
			throw std::runtime_error("Failed to allocate image memory: "s + e.what());
		}

		VulkanRenderer::GetDevice().bindImageMemory(m_Image, m_ImageMemory, 0);
	}

	vk::ImageView VulkanTexture::CreateImageView(vk::ImageAspectFlags aspectFlags)
	{
		vk::ImageViewType viewType = m_TextureMode == TextureMode::Cubemap
			? vk::ImageViewType::eCube
			: vk::ImageViewType::e2D;

		vk::ImageViewCreateInfo viewInfo;
		viewInfo.image = m_Image;
		viewInfo.viewType = viewType;
		viewInfo.format = m_Format;
		viewInfo.subresourceRange.aspectMask = aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = m_LayerCount;

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

	void VulkanTexture::CopyBufferToImage(vk::Buffer buffer)
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
		region.imageExtent = vk::Extent3D(m_Width, m_Height, 1);

		commandBuffer.copyBufferToImage(buffer, m_Image, vk::ImageLayout::eTransferDstOptimal, region);

		VulkanHelpers::EndSingleTimeCommands(commandBuffer);
	}

	void VulkanTexture::CopyBufferToImageCubemap(vk::Buffer buffer)
	{
		vk::CommandBuffer cmd = VulkanHelpers::BeginSingleTimeCommands();

		std::vector<vk::BufferImageCopy> regions;

		for (uint32_t face = 0; face < 6; face++)
		{
			// TODO: add support for mip levels.

			const uint32_t imageSize = m_Width * m_Height * 4 * 6;
			const uint32_t layerSize = imageSize / 6;
			const uint32_t offset = layerSize * face;

			vk::BufferImageCopy region{};
			region.bufferOffset = offset;
			region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
			region.imageSubresource.mipLevel = 0;
			region.imageSubresource.baseArrayLayer = face;
			region.imageSubresource.layerCount = 1;
			region.imageExtent.width = m_Width;
			region.imageExtent.height = m_Height;
			region.imageExtent.depth = 1;
			regions.push_back(region);
		}

		cmd.copyBufferToImage(buffer, m_Image, vk::ImageLayout::eTransferDstOptimal, regions);
		VulkanHelpers::EndSingleTimeCommands(cmd);
	}
}
