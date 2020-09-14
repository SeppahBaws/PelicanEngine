#pragma once
#include "Vertex.h"

#include <vulkan/vulkan.h>

namespace Pelican
{
	class Mesh
	{
	public:
		Mesh(std::vector<Vertex> vertices, std::vector<uint16_t> indices);
		void Cleanup();

		void CreateBuffers();
		void Draw();

	private:
		std::vector<Vertex> m_Vertices;
		std::vector<uint16_t> m_Indices;

		VkBuffer m_VkVertexBuffer{};
		VkDeviceMemory m_VkVertexBufferMemory{};
		VkBuffer m_VkIndexBuffer{};
		VkDeviceMemory m_VkIndexBufferMemory{};
	};
}
