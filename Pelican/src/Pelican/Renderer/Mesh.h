#pragma once
#include "Vertex.h"

#include <vulkan/vulkan.hpp>

#include "Camera.h"

namespace Pelican
{
	class GltfModel;
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
		Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices, int32_t materialIdx);
		void Cleanup();

		void SetupVerticesIndices(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

		void CreateBuffers();
		void CreateDescriptorSet(const GltfModel* pParent, const vk::DescriptorPool& pool);

		void Update(const glm::mat4& model, const glm::mat4& view, const glm::mat4& proj);
		void Draw() const;


	private:
		std::vector<Vertex> m_Vertices{};
		std::vector<uint32_t> m_Indices{};
		int32_t m_MaterialIdx;

		vk::Buffer m_UniformBuffer{};
		vk::DeviceMemory m_UniformBufferMemory{};
		vk::Buffer m_LightBuffer{};
		vk::DeviceMemory m_LightBufferMemory{};

		vk::Buffer m_VertexBuffer{};
		vk::DeviceMemory m_VertexBufferMemory{};
		vk::Buffer m_IndexBuffer{};
		vk::DeviceMemory m_IndexBufferMemory{};
		vk::DescriptorSet m_DescriptorSet{};
	};
}
