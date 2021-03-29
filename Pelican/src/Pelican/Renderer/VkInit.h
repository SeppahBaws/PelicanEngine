#pragma once

#include <vulkan/vulkan.hpp>

namespace Pelican
{
	namespace VkInit
	{
		//-----------------------------------------------------------
		// Graphics pipeline and render pass related infos
		//-----------------------------------------------------------

		vk::PipelineRasterizationStateCreateInfo RasterizationStateCreateInfo(vk::PolygonMode polygonMode, vk::CullModeFlags cullMode);
		vk::PipelineMultisampleStateCreateInfo MultisampleStateCreateInfo();
		vk::PipelineDepthStencilStateCreateInfo DepthStencilCreateInfo(bool depthTest, bool depthWrite, vk::CompareOp compareOp);


		//-----------------------------------------------------------
		// Image related infos
		//-----------------------------------------------------------

		vk::ImageMemoryBarrier ImageMemoryBarrier(vk::Image image, vk::ImageLayout oldLayout = {}, vk::ImageLayout newLayout = {});

	}
}
