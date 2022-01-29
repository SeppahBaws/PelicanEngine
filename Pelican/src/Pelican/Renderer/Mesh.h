#pragma once
#include "Vertex.h"

#include <vulkan/vulkan.hpp>

#include "Camera.h"
#include "VulkanBuffer.h"

namespace Pelican
{
	class Model;
	class Camera;
	class VulkanTexture;

	enum class TextureSlot : uint32_t
	{
		ALBEDO = 0,
		NORMAL,
		METALLIC_ROUGHNESS,
		AMBIENT_OCCLUSION,

		SLOT_COUNT
	};

	class Mesh
	{
	public:
		Mesh() = default;
		Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices, uint32_t materialIdx);
		void Cleanup();

		void SetupVerticesIndices(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

		void CreateBuffers();
		void CreateDescriptorSet(const Model* pParent, const vk::DescriptorPool& pool);

		void Update(const glm::mat4& model, const glm::mat4& view, const glm::mat4& proj);
		void Draw() const;


	private:
		std::vector<Vertex> m_Vertices{};
		std::vector<uint32_t> m_Indices{};
		uint32_t m_MaterialIdx;

		VulkanBuffer* m_pUniformBuffer;
		VulkanBuffer* m_pLightBuffer;

		VulkanBuffer* m_pVertexBuffer;
		VulkanBuffer* m_pIndexBuffer;

		vk::DescriptorSet m_DescriptorSet{};
	};
}
