#pragma once
#include "Vertex.h"

#include <vulkan/vulkan.h>

namespace Pelican
{
	class Mesh
	{
	public:
		Mesh();
		Mesh(const std::string& filename);
		Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices);
		void Cleanup();

		void SetupVerticesIndices(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

		void CreateBuffers();
		void Draw();

	private:
		std::vector<Vertex> m_Vertices;
		std::vector<uint32_t> m_Indices;

		VkBuffer m_VkVertexBuffer{};
		VkDeviceMemory m_VkVertexBufferMemory{};
		VkBuffer m_VkIndexBuffer{};
		VkDeviceMemory m_VkIndexBufferMemory{};
	};
}
