#pragma once
#include "Vertex.h"

#include <vulkan/vulkan.h>

namespace Pelican
{
	class Camera;
	class VulkanTexture;

	class Mesh
	{
	public:
		Mesh();
		Mesh(const std::string& filename);
		Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices);
		void Cleanup();

		void SetupVerticesIndices(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
		void SetupTexture(const std::string& path);

		void CreateBuffers();
		void CreateDescriptorSet(const VkDescriptorPool& pool);

		void Update(Camera* pCamera);
		void Draw() const;

	private:
		std::vector<Vertex> m_Vertices;
		std::vector<uint32_t> m_Indices;

		VkBuffer m_UniformBuffer{};
		VkDeviceMemory m_UniformBufferMemory{};

		VkBuffer m_VkVertexBuffer{};
		VkDeviceMemory m_VkVertexBufferMemory{};
		VkBuffer m_VkIndexBuffer{};
		VkDeviceMemory m_VkIndexBufferMemory{};
		VkDescriptorSet m_DescriptorSet{};

		VulkanTexture* m_pTexture{};
	};
}
