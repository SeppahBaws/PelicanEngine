#include "PelicanPCH.h"
#include "VkInit.h"

namespace Pelican
{
	vk::PipelineRasterizationStateCreateInfo VkInit::RasterizationStateCreateInfo(vk::PolygonMode polygonMode,
		vk::CullModeFlags cullMode)
	{
		vk::PipelineRasterizationStateCreateInfo info = vk::PipelineRasterizationStateCreateInfo()
			.setDepthClampEnable(false)
			.setRasterizerDiscardEnable(false)
			.setPolygonMode(polygonMode)
			.setLineWidth(1.0f)
			.setCullMode(cullMode)
			.setFrontFace(vk::FrontFace::eCounterClockwise)
			.setDepthBiasEnable(false)
			.setDepthBiasConstantFactor(0.0f)
			.setDepthBiasClamp(0.0f)
			.setDepthBiasSlopeFactor(0.0f);

		return info;
	}

	vk::PipelineMultisampleStateCreateInfo VkInit::MultisampleStateCreateInfo()
	{
		vk::PipelineMultisampleStateCreateInfo info = vk::PipelineMultisampleStateCreateInfo()
			.setSampleShadingEnable(false)
			.setRasterizationSamples(vk::SampleCountFlagBits::e1)
			.setMinSampleShading(1.0f)
			.setPSampleMask(nullptr)
			.setAlphaToCoverageEnable(false)
			.setAlphaToOneEnable(false);

		return info;
	}

	vk::PipelineDepthStencilStateCreateInfo VkInit::DepthStencilCreateInfo(bool depthTest, bool depthWrite,
		vk::CompareOp compareOp)
	{
		vk::PipelineDepthStencilStateCreateInfo info = vk::PipelineDepthStencilStateCreateInfo()
			.setDepthTestEnable(depthTest)
			.setDepthWriteEnable(depthWrite)
			.setDepthCompareOp(depthTest ? compareOp : vk::CompareOp::eAlways)
			.setDepthBoundsTestEnable(false)
			.setMinDepthBounds(0.0f)
			.setMaxDepthBounds(1.0f)
			.setStencilTestEnable(false);

		return info;
	}

	vk::ImageMemoryBarrier VkInit::ImageMemoryBarrier(vk::Image image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout)
	{
		vk::ImageMemoryBarrier barrier = vk::ImageMemoryBarrier()
			.setOldLayout(oldLayout)
			.setNewLayout(newLayout)
			.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
			.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
			.setImage(image)
			.setSubresourceRange(vk::ImageSubresourceRange()
				.setAspectMask(vk::ImageAspectFlagBits::eColor)
				.setBaseMipLevel(0)
				.setLevelCount(1)
				.setBaseArrayLayer(0)
				.setLayerCount(1));

		return barrier;
	}
}
