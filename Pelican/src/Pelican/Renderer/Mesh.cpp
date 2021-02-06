#include "PelicanPCH.h"
#include "Mesh.h"

#include "Pelican/Renderer/Camera.h"
#include "VulkanHelpers.h"
#include "VulkanRenderer.h"
#include "VulkanTexture.h"

#include <logtools.h>

#pragma warning(push)
#pragma warning(disable:4996)
#pragma warning(disable:4840)
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_NOEXCEPTION
#define JSON_NOEXCEPTION
#include "tiny_gltf.h"
#pragma warning(pop)

#pragma warning(push)
#pragma warning(disable:4201)
#include <glm/gtc/type_ptr.hpp>

#include "UniformBufferObject.h"
#pragma warning(pop)

namespace tinygltf
{
	static inline int32_t GetTypeSizeInBytes(uint32_t ty) {
		if (ty == TINYGLTF_TYPE_SCALAR) {
			return 1;
		}
		else if (ty == TINYGLTF_TYPE_VEC2) {
			return 2;
		}
		else if (ty == TINYGLTF_TYPE_VEC3) {
			return 3;
		}
		else if (ty == TINYGLTF_TYPE_VEC4) {
			return 4;
		}
		else if (ty == TINYGLTF_TYPE_MAT2) {
			return 4;
		}
		else if (ty == TINYGLTF_TYPE_MAT3) {
			return 9;
		}
		else if (ty == TINYGLTF_TYPE_MAT4) {
			return 16;
		}
		else {
			// Unknown componenty type
			return -1;
		}
	}

}

namespace Pelican
{
	Mesh::Mesh()
	{
		// Empty texture as fallback.
		m_pWhiteTexture = new VulkanTexture("res/textures/white.png");

		// Setup defaults for the texture slots
		for (uint32_t i = 0; i < static_cast<uint32_t>(TextureSlot::SLOT_COUNT); i++)
		{
			m_pTextures[i] = m_pWhiteTexture;
		}
	}

	Mesh::Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices)
		: m_Vertices(std::move(vertices)), m_Indices(std::move(indices))
	{
		// Empty texture as fallback.
		m_pWhiteTexture = new VulkanTexture("res/textures/white.png");

		// Setup defaults for the texture slots
		for (uint32_t i = 0; i < static_cast<uint32_t>(TextureSlot::SLOT_COUNT); i++)
		{
			m_pTextures[i] = m_pWhiteTexture;
		}
	}

	void Mesh::Cleanup()
	{
		for (uint32_t i = 0; i < static_cast<uint32_t>(TextureSlot::SLOT_COUNT); i++)
		{
			if (m_pTextures[i])
			{
				if (m_pTextures[i] != m_pWhiteTexture)
					delete m_pTextures[i];
				m_pTextures[i] = nullptr;
			}
		}

		delete m_pWhiteTexture;
		m_pWhiteTexture = nullptr;

		vkDestroyBuffer(VulkanRenderer::GetDevice(), m_VkIndexBuffer, nullptr);
		vkFreeMemory(VulkanRenderer::GetDevice(), m_VkIndexBufferMemory, nullptr);

		vkDestroyBuffer(VulkanRenderer::GetDevice(), m_VkVertexBuffer, nullptr);
		vkFreeMemory(VulkanRenderer::GetDevice(), m_VkVertexBufferMemory, nullptr);

		vkDestroyBuffer(VulkanRenderer::GetDevice(), m_UniformBuffer, nullptr);
		vkFreeMemory(VulkanRenderer::GetDevice(), m_UniformBufferMemory, nullptr);
	}

	void Mesh::SetupVerticesIndices(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
	{
		m_Vertices = vertices;
		m_Indices = indices;

		CreateBuffers();
		// CreateDescriptorSet();
	}

	void Mesh::SetupTexture(TextureSlot slot, const std::string& path)
	{
		m_pTextures[static_cast<uint32_t>(slot)] = new VulkanTexture(path);
	}

	VulkanTexture* Mesh::GetTexture(TextureSlot slot)
	{
		return m_pTextures[static_cast<uint32_t>(slot)];
	}

	void Mesh::Update(const glm::mat4& model, const glm::mat4& view, const glm::mat4& proj)
	{
		UniformBufferObject ubo{};
		ubo.model = model;
		ubo.view = view;
		ubo.proj = proj;

		void* data;
		vkMapMemory(VulkanRenderer::GetDevice(), m_UniformBufferMemory, 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(VulkanRenderer::GetDevice(), m_UniformBufferMemory);
	}

	void Mesh::Draw() const
	{
		VkCommandBuffer commandBuffer = VulkanRenderer::GetCurrentBuffer();

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, VulkanRenderer::GetPipelineLayout(), 0, 1, &m_DescriptorSet, 0, nullptr);

		VkBuffer vertexBuffers[] = { m_VkVertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(commandBuffer, m_VkIndexBuffer, 0, VK_INDEX_TYPE_UINT32);

		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(m_Indices.size()), 1, 0, 0, 0);
	}

	void Mesh::CreateBuffers()
	{
		// Uniform buffer
		{
			VkDeviceSize bufferSize = sizeof(UniformBufferObject);
			VulkanHelpers::CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				m_UniformBuffer, m_UniformBufferMemory);
		}

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

	void Mesh::CreateDescriptorSet(const VkDescriptorPool& pool)
	{
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = m_UniformBuffer;
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(UniformBufferObject);
		
		VkDescriptorImageInfo imageInfos[static_cast<uint32_t>(TextureSlot::SLOT_COUNT)] =
		{
			GetTexture(TextureSlot::ALBEDO)->GetDescriptorImageInfo(),
			GetTexture(TextureSlot::NORMAL)->GetDescriptorImageInfo(),
			GetTexture(TextureSlot::METALLIC_ROUGHNESS)->GetDescriptorImageInfo(),
		};

		VkDescriptorSetAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
		allocInfo.descriptorPool = pool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &VulkanRenderer::GetDescriptorSetLayout();

		const VkResult result = vkAllocateDescriptorSets(VulkanRenderer::GetDevice(), &allocInfo, &m_DescriptorSet);
		ASSERT_MSG(result == VK_SUCCESS, "failed to allocate descriptor sets!");

		std::array<VkWriteDescriptorSet, 4> descriptorWrites{};

		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = m_DescriptorSet;
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &bufferInfo;

		// Albedo texture
		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = m_DescriptorSet;
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pImageInfo = &imageInfos[0];

		// Normal texture
		descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[2].dstSet = m_DescriptorSet;
		descriptorWrites[2].dstBinding = 2;
		descriptorWrites[2].dstArrayElement = 0;
		descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[2].descriptorCount = 1;
		descriptorWrites[2].pImageInfo = &imageInfos[1];

		// Metallic Roughness texture
		descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[3].dstSet = m_DescriptorSet;
		descriptorWrites[3].dstBinding = 3;
		descriptorWrites[3].dstArrayElement = 0;
		descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[3].descriptorCount = 1;
		descriptorWrites[3].pImageInfo = &imageInfos[2];

		vkUpdateDescriptorSets(VulkanRenderer::GetDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
}
