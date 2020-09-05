#include "PelicanPCH.h"
#include "Mesh.h"

#include "VulkanHelpers.h"
#include "VulkanRenderer.h"

namespace Pelican
{
	Mesh::Mesh(std::vector<Vertex> vertices, std::vector<uint16_t> indices)
		: m_Vertices(std::move(vertices)), m_Indices(std::move(indices))
	{
	}

	void Mesh::Cleanup()
	{
		vkDestroyBuffer(VulkanRenderer::GetDevice(), m_VkIndexBuffer, nullptr);
		vkFreeMemory(VulkanRenderer::GetDevice(), m_VkIndexBufferMemory, nullptr);

		vkDestroyBuffer(VulkanRenderer::GetDevice(), m_VkVertexBuffer, nullptr);
		vkFreeMemory(VulkanRenderer::GetDevice(), m_VkVertexBufferMemory, nullptr);
	}

	void Mesh::Draw()
	{
		VkCommandBuffer commandBuffer = VulkanRenderer::GetCurrentBuffer();

		VkBuffer vertexBuffers[] = { m_VkVertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(commandBuffer, m_VkIndexBuffer, 0, VK_INDEX_TYPE_UINT16);

		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(m_Indices.size()), 1, 0, 0, 0);
	}

	void Mesh::CreateBuffers()
	{
		// Vertex Buffer
		{
			VkDeviceSize bufferSize = sizeof(m_Vertices[0]) * m_Vertices.size();

			VkBuffer stagingBuffer;
			VkDeviceMemory stagingBufferMemory;
			VulkanHelpers::CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				stagingBuffer, stagingBufferMemory);

			void* data;
			vkMapMemory(VulkanRenderer::GetDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
			memcpy(data, m_Vertices.data(), static_cast<size_t>(bufferSize));
			vkUnmapMemory(VulkanRenderer::GetDevice(), stagingBufferMemory);

			VulkanHelpers::CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				m_VkVertexBuffer, m_VkVertexBufferMemory);

			VulkanHelpers::CopyBuffer(stagingBuffer, m_VkVertexBuffer, bufferSize);

			vkDestroyBuffer(VulkanRenderer::GetDevice(), stagingBuffer, nullptr);
			vkFreeMemory(VulkanRenderer::GetDevice(), stagingBufferMemory, nullptr);
		}

		// Index Buffer
		{
			VkDeviceSize bufferSize = sizeof(m_Indices[0]) * m_Indices.size();

			VkBuffer stagingBuffer;
			VkDeviceMemory stagingBufferMemory;
			VulkanHelpers::CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				stagingBuffer, stagingBufferMemory);

			void* data;
			vkMapMemory(VulkanRenderer::GetDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
			memcpy(data, m_Indices.data(), static_cast<size_t>(bufferSize));
			vkUnmapMemory(VulkanRenderer::GetDevice(), stagingBufferMemory);

			VulkanHelpers::CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				m_VkIndexBuffer, m_VkIndexBufferMemory);

			VulkanHelpers::CopyBuffer(stagingBuffer, m_VkIndexBuffer, bufferSize);

			vkDestroyBuffer(VulkanRenderer::GetDevice(), stagingBuffer, nullptr);
			vkFreeMemory(VulkanRenderer::GetDevice(), stagingBufferMemory, nullptr);
		}
	}
}
