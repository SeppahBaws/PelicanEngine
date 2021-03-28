#pragma once
#include "Vertex.h"

#include <vulkan/vulkan.hpp>

#include "Camera.h"

namespace Pelican
{
	class Camera;
	class VulkanTexture;

	enum class TextureSlot : uint32_t
	{
		ALBEDO = 0,
		NORMAL,
		METALLIC_ROUGHNESS,

		SLOT_COUNT
	};

	class Mesh
	{
	public:
		Mesh();
		Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices);
		void Cleanup();

		void SetupVerticesIndices(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

		void SetupTexture(TextureSlot slot, const std::string& path);
		VulkanTexture* GetTexture(TextureSlot slot);

		void CreateBuffers();
		void CreateDescriptorSet(const vk::DescriptorPool& pool);

		void Update(const glm::mat4& model, const glm::mat4& view, const glm::mat4& proj);
		void Draw() const;


	private:
		std::vector<Vertex> m_Vertices{};
		std::vector<uint32_t> m_Indices{};

		vk::Buffer m_UniformBuffer{};
		vk::DeviceMemory m_UniformBufferMemory{};

		vk::Buffer m_VertexBuffer{};
		vk::DeviceMemory m_VertexBufferMemory{};
		vk::Buffer m_IndexBuffer{};
		vk::DeviceMemory m_IndexBufferMemory{};
		vk::DescriptorSet m_DescriptorSet{};

		// VulkanTexture* m_pTexture{};
		VulkanTexture* m_pTextures[static_cast<uint32_t>(TextureSlot::SLOT_COUNT)]{};
		VulkanTexture* m_pWhiteTexture{};
	};
}
