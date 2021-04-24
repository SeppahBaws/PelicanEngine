#include "PelicanPCH.h"
#include "VulkanPipeline.h"


#include "Vertex.h"
#include "VkInit.h"
#include "VulkanShader.h"

namespace Pelican
{
	void VulkanPipeline::Init(const vk::Pipeline& pipeline, const vk::PipelineCache& cache,
		const vk::PipelineLayout& layout)
	{
		m_Pipeline = pipeline;
		m_Cache = cache;
		m_Layout = layout;
	}

	void VulkanPipeline::Cleanup(vk::Device device) const
	{
		device.destroyPipeline(m_Pipeline);
		device.destroyPipelineCache(m_Cache);
		device.destroyPipelineLayout(m_Layout);
	}

	PipelineBuilder::PipelineBuilder(vk::Device device)
		: m_Device(device)
	{
	}

	void PipelineBuilder::SetShader(VulkanShader* pShader)
	{
		m_pShader = pShader;
	}

	void PipelineBuilder::SetInputAssembly(vk::PrimitiveTopology topology, bool primitiveRestartEnable)
	{
		m_InputAssembly = vk::PipelineInputAssemblyStateCreateInfo()
			.setTopology(topology)
			.setPrimitiveRestartEnable(primitiveRestartEnable);
	}

	void PipelineBuilder::SetViewport(float x, float y, float width, float height, float minDepth, float maxDepth)
	{
		m_Viewport = vk::Viewport()
			.setX(x).setY(y)
			.setWidth(width).setHeight(height)
			.setMinDepth(minDepth).setMaxDepth(maxDepth);
	}

	void PipelineBuilder::SetScissor(const vk::Offset2D& offset, const vk::Extent2D& extent)
	{
		m_Scissor = vk::Rect2D()
			.setOffset(offset)
			.setExtent(extent);
	}

	void PipelineBuilder::SetRasterizer(vk::PolygonMode polygonMode, vk::CullModeFlagBits cullMode)
	{
		m_Rasterizer = VkInit::RasterizationStateCreateInfo(polygonMode, cullMode);
	}

	void PipelineBuilder::SetMultisampling()
	{
		m_Multisampling = VkInit::MultisampleStateCreateInfo();
	}

	void PipelineBuilder::SetDepthStencil(bool depthTest, bool depthWrite, vk::CompareOp compareOp)
	{
		m_DepthStencil = VkInit::DepthStencilCreateInfo(depthTest, depthWrite, compareOp);
	}

	void PipelineBuilder::SetColorBlend(bool blendEnable, vk::BlendOp colorBlendOp, vk::BlendOp alphaBlendOp,
		bool logicOpEnable, vk::LogicOp logicOp)
	{
		m_ColorBlendAttachment = vk::PipelineColorBlendAttachmentState()
			.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA)
			.setBlendEnable(blendEnable)
			.setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
			.setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
			.setColorBlendOp(colorBlendOp)
			.setSrcAlphaBlendFactor(vk::BlendFactor::eSrcAlpha)
			.setDstAlphaBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
			.setAlphaBlendOp(alphaBlendOp);

		m_ColorBlending = vk::PipelineColorBlendStateCreateInfo()
			.setLogicOpEnable(logicOpEnable)
			.setLogicOp(logicOp)
			.setAttachments(m_ColorBlendAttachment)
			.setBlendConstants({ 0.0f, 0.0f, 0.0f, 0.0f });
	}

	void PipelineBuilder::SetDescriptorSetLayout(uint32_t count, const vk::DescriptorSetLayout* pLayouts)
	{
		m_PipelineLayoutInfo = vk::PipelineLayoutCreateInfo()
			.setSetLayoutCount(count)
			.setPSetLayouts(pLayouts);

		// We'll ignore push constants for now. TODO: integrate push constants
			// .setPushConstantRanges({});
	}

	VulkanPipeline PipelineBuilder::BuildGraphics(const vk::RenderPass& renderPass)
	{
		auto bindingDescription = Vertex::GetBindingDescription();
		auto attributeDescriptions = Vertex::GetAttributeDescriptions();

		const vk::PipelineVertexInputStateCreateInfo vertexInputInfo = vk::PipelineVertexInputStateCreateInfo()
			.setVertexBindingDescriptions(bindingDescription)
			.setVertexAttributeDescriptions(attributeDescriptions);

		const vk::PipelineViewportStateCreateInfo viewportState = vk::PipelineViewportStateCreateInfo()
			.setViewports(m_Viewport)
			.setScissors(m_Scissor);

		// First we need to create the pipeline layout.
		vk::PipelineLayout pipelineLayout;

		try
		{
			pipelineLayout = m_Device.createPipelineLayout(m_PipelineLayoutInfo);
		}
		catch (vk::SystemError& e)
		{
			throw std::runtime_error("Failed to create pipeline layout: "s + e.what());
		}

		std::vector<vk::PipelineShaderStageCreateInfo> shaderStages = m_pShader->GetAllShaderStages();
		const vk::GraphicsPipelineCreateInfo pipelineInfo = vk::GraphicsPipelineCreateInfo()
			.setStages(shaderStages)

			.setPVertexInputState(&vertexInputInfo)
			.setPInputAssemblyState(&m_InputAssembly)
			.setPViewportState(&viewportState)
			.setPRasterizationState(&m_Rasterizer)
			.setPMultisampleState(&m_Multisampling)
			.setPDepthStencilState(&m_DepthStencil)
			.setPColorBlendState(&m_ColorBlending)
			.setPDynamicState(nullptr)

			.setLayout(pipelineLayout)
			.setRenderPass(renderPass)
			.setSubpass(0)

			.setBasePipelineHandle(nullptr)
			.setBasePipelineIndex(-1);

		const vk::PipelineCache pipelineCache = m_Device.createPipelineCache(vk::PipelineCacheCreateInfo());

		const vk::ResultValue<vk::Pipeline> pipelineResult = m_Device.createGraphicsPipeline(pipelineCache, pipelineInfo);

		if (pipelineResult.result != vk::Result::eSuccess)
		{
			throw std::runtime_error("Failed to create graphics pipeline");
		}

		VulkanPipeline pipeline;
		pipeline.Init(pipelineResult.value, pipelineCache, pipelineLayout);
		return pipeline;
	}

	VulkanPipeline PipelineBuilder::BuildCompute()
	{
		vk::PipelineLayout pipelineLayout;

		try
		{
			pipelineLayout = m_Device.createPipelineLayout(m_PipelineLayoutInfo);
		}
		catch (vk::SystemError& e)
		{
			throw std::runtime_error("Failed to create pipeline layout: "s + e.what());
		}

		const vk::ComputePipelineCreateInfo pipelineInfo = vk::ComputePipelineCreateInfo()
			.setStage(m_pShader->GetShaderStage(ShaderType::Compute))
			.setLayout(pipelineLayout)
			.setBasePipelineHandle(nullptr)
			.setBasePipelineIndex(-1);

		const vk::PipelineCache pipelineCache = m_Device.createPipelineCache(vk::PipelineCacheCreateInfo());

		const vk::ResultValue<vk::Pipeline> pipelineResult = m_Device.createComputePipeline(pipelineCache, pipelineInfo);
		if (pipelineResult.result != vk::Result::eSuccess)
		{
			throw std::runtime_error("Failed to create compute pipeline");
		}

		VulkanPipeline pipeline;
		pipeline.Init(pipelineResult.value, pipelineCache, pipelineLayout);
		return pipeline;
	}
}
