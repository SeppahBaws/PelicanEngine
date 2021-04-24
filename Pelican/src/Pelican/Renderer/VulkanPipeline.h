#pragma once

#include <vulkan/vulkan.hpp>

namespace Pelican
{
	class VulkanShader;

	class VulkanPipeline
	{
	public:
		VulkanPipeline() = default;

		void Init(const vk::Pipeline& pipeline, const vk::PipelineCache& cache, const vk::PipelineLayout& layout);
		void Cleanup(vk::Device device) const;

		[[nodiscard]] const vk::Pipeline& GetPipeline() const { return m_Pipeline; }
		[[nodiscard]] const vk::PipelineCache& GetCache() const { return m_Cache; }
		[[nodiscard]] const vk::PipelineLayout& GetLayout() const { return m_Layout; }

	private:
		vk::Pipeline m_Pipeline;
		vk::PipelineCache m_Cache;
		vk::PipelineLayout m_Layout;
	};

	class PipelineBuilder
	{
	public:
		PipelineBuilder(vk::Device device);

		void SetShader(VulkanShader* pShader);
		void SetInputAssembly(vk::PrimitiveTopology topology, bool primitiveRestartEnable);
		void SetViewport(float x, float y, float width, float height, float minDepth, float maxDepth);
		void SetScissor(const vk::Offset2D& offset, const vk::Extent2D& extent);
		void SetRasterizer(vk::PolygonMode polygonMode, vk::CullModeFlagBits cullMode);
		void SetMultisampling();
		void SetDepthStencil(bool depthTest, bool depthWrite, vk::CompareOp compareOp);
		void SetColorBlend(bool blendEnable, vk::BlendOp colorBlendOp, vk::BlendOp alphaBlendOp, bool logicOpEnable, vk::LogicOp logicOp);
		void SetDescriptorSetLayout(uint32_t count, const vk::DescriptorSetLayout* pLayouts);

		VulkanPipeline BuildGraphics(const vk::RenderPass& renderPass);

	private:
		vk::Device m_Device;

		VulkanShader* m_pShader{};
		vk::PipelineInputAssemblyStateCreateInfo m_InputAssembly{};
		vk::Viewport m_Viewport{};
		vk::Rect2D m_Scissor{};
		vk::PipelineRasterizationStateCreateInfo m_Rasterizer{};
		vk::PipelineMultisampleStateCreateInfo m_Multisampling{};
		vk::PipelineDepthStencilStateCreateInfo m_DepthStencil{};
		vk::PipelineColorBlendAttachmentState m_ColorBlendAttachment{};
		vk::PipelineColorBlendStateCreateInfo m_ColorBlending{};
		vk::PipelineLayoutCreateInfo m_PipelineLayoutInfo{};
	};
}
