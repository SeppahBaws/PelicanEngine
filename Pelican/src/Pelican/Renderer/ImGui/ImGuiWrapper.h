#pragma once

#include <glm/vec2.hpp>
#include <vulkan/vulkan.h>

namespace Pelican
{
	class VulkanDevice;
	class VulkanShader;

	class ImGuiWrapper
	{
	public:
		struct PushConstBlock
		{
			glm::vec2 scale;
			glm::vec2 translate;
		} m_pushConstBlock;

		ImGuiWrapper(VulkanDevice* pDevice);
		~ImGuiWrapper();

		void Init(float width, float height);
		void InitResources(VkRenderPass renderPass, VkQueue copyQueue, const std::string& shadersPath);

		// Handle ImGui IO
		void UpdateIO();

		void NewFrame();
		void Render();
		void UpdateBuffers();
		void DrawFrame();

	private:
		VkSampler m_Sampler{};
		VkBuffer m_VertexBuffer{};
		VkDeviceMemory m_VertexBufferMemory{};
		void* m_VBMapped{};
		VkBuffer m_IndexBuffer{};
		VkDeviceMemory m_IndexBufferMemory{};
		void* m_IBMapped{};
		int32_t m_VertexCount{};
		int32_t m_IndexCount{};
		VkDeviceMemory m_FontMemory{};
		VkImage m_FontImage{};
		VkImageView m_FontImageView{};
		VkPipelineCache m_PipelineCache{};
		VkPipelineLayout m_PipelineLayout{};
		VkPipeline m_Pipeline{};
		VkDescriptorPool m_DescriptorPool{};
		VkDescriptorSetLayout m_DescriptorSetLayout{};
		VkDescriptorSet m_DescriptorSet{};
		VulkanDevice* m_pDevice{};

		VulkanShader* m_pShader{};
	};
}
