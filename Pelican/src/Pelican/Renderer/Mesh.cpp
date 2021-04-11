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
#include "Pelican/Assets/AssetManager.h"
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
		m_pWhiteTexture = AssetManager::GetInstance().LoadTexture("res/textures/white.png");

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
		m_pWhiteTexture = AssetManager::GetInstance().LoadTexture("res/textures/white.png");

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
					AssetManager::GetInstance().UnloadTexture(m_pTextures[i]);
				m_pTextures[i] = nullptr;
			}
		}

		AssetManager::GetInstance().UnloadTexture(m_pWhiteTexture);
		m_pWhiteTexture = nullptr;

		const vk::Device device = VulkanRenderer::GetDevice();
		device.destroyBuffer(m_IndexBuffer);
		device.freeMemory(m_IndexBufferMemory);

		device.destroyBuffer(m_VertexBuffer);
		device.freeMemory(m_VertexBufferMemory);

		device.destroyBuffer(m_UniformBuffer);
		device.freeMemory(m_UniformBufferMemory);
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
		m_pTextures[static_cast<uint32_t>(slot)] = AssetManager::GetInstance().LoadTexture(path);
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


		void* data = VulkanRenderer::GetDevice().mapMemory(m_UniformBufferMemory, 0, sizeof(ubo));
			memcpy(data, &ubo, sizeof(ubo));
		VulkanRenderer::GetDevice().unmapMemory(m_UniformBufferMemory);
	}

	void Mesh::Draw() const
	{
		vk::CommandBuffer commandBuffer = VulkanRenderer::GetCurrentBuffer();

		commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, VulkanRenderer::GetPipelineLayout(), 0, m_DescriptorSet, {});

		std::vector<vk::Buffer> vertexBuffers = { m_VertexBuffer };
		std::vector<vk::DeviceSize> offsets = { 0 };
		commandBuffer.bindVertexBuffers(0, vertexBuffers, offsets);
		commandBuffer.bindIndexBuffer(m_IndexBuffer, 0, vk::IndexType::eUint32);

		commandBuffer.drawIndexed(static_cast<uint32_t>(m_Indices.size()), 1, 0, 0, 0);
	}

	void Mesh::CreateBuffers()
	{
		// Uniform buffer
		{
			const vk::DeviceSize bufferSize = sizeof(UniformBufferObject);
			VulkanHelpers::CreateBuffer(
				bufferSize,
				vk::BufferUsageFlagBits::eUniformBuffer,
				vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
				m_UniformBuffer, m_UniformBufferMemory);
		}

		// Vertex Buffer
		{
			const vk::DeviceSize bufferSize = sizeof(m_Vertices[0]) * m_Vertices.size();

			vk::Buffer stagingBuffer;
			vk::DeviceMemory stagingBufferMemory;
			VulkanHelpers::CreateBuffer(
				bufferSize,
				vk::BufferUsageFlagBits::eTransferSrc,
				vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
				stagingBuffer, stagingBufferMemory);

			void* data = VulkanRenderer::GetDevice().mapMemory(stagingBufferMemory, 0, bufferSize);
				memcpy(data, m_Vertices.data(), static_cast<size_t>(bufferSize));
			VulkanRenderer::GetDevice().unmapMemory(stagingBufferMemory);

			VulkanHelpers::CreateBuffer(
				bufferSize,
				vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
				vk::MemoryPropertyFlagBits::eDeviceLocal,
				m_VertexBuffer, m_VertexBufferMemory);

			VulkanHelpers::CopyBuffer(stagingBuffer, m_VertexBuffer, bufferSize);

			VulkanRenderer::GetDevice().destroyBuffer(stagingBuffer);
			VulkanRenderer::GetDevice().freeMemory(stagingBufferMemory);
		}

		// Index Buffer
		{
			const vk::DeviceSize bufferSize = sizeof(m_Indices[0]) * m_Indices.size();

			vk::Buffer stagingBuffer;
			vk::DeviceMemory stagingBufferMemory;
			VulkanHelpers::CreateBuffer(
				bufferSize,
				vk::BufferUsageFlagBits::eTransferSrc,
				vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
				stagingBuffer, stagingBufferMemory);

			void* data = VulkanRenderer::GetDevice().mapMemory(stagingBufferMemory, 0, bufferSize);
				memcpy(data, m_Indices.data(), static_cast<size_t>(bufferSize));
			VulkanRenderer::GetDevice().unmapMemory(stagingBufferMemory);

			VulkanHelpers::CreateBuffer(
				bufferSize,
				vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
				vk::MemoryPropertyFlagBits::eDeviceLocal,
				m_IndexBuffer, m_IndexBufferMemory);

			VulkanHelpers::CopyBuffer(stagingBuffer, m_IndexBuffer, bufferSize);

			VulkanRenderer::GetDevice().destroyBuffer(stagingBuffer);
			VulkanRenderer::GetDevice().freeMemory(stagingBufferMemory);
		}
	}

	void Mesh::CreateDescriptorSet(const vk::DescriptorPool& pool)
	{
		vk::DescriptorBufferInfo bufferInfo(m_UniformBuffer, 0, sizeof(UniformBufferObject));

		std::array<vk::DescriptorImageInfo, static_cast<uint32_t>(TextureSlot::SLOT_COUNT)> imageInfos =
		{
			GetTexture(TextureSlot::ALBEDO)->GetDescriptorImageInfo(),
			GetTexture(TextureSlot::NORMAL)->GetDescriptorImageInfo(),
			GetTexture(TextureSlot::METALLIC_ROUGHNESS)->GetDescriptorImageInfo(),
		};

		vk::DescriptorSetAllocateInfo allocInfo;
		allocInfo.descriptorPool = pool;
		allocInfo.setSetLayouts(VulkanRenderer::GetDescriptorSetLayout());

		try
		{
			std::vector<vk::DescriptorSet> sets = VulkanRenderer::GetDevice().allocateDescriptorSets(allocInfo);

			ASSERT_MSG(!sets.empty(), "No descriptors were allocated!");

			m_DescriptorSet = sets[0];
		}
		catch (vk::SystemError& e)
		{
			throw std::runtime_error("Failed to allocate descriptor sets: "s + e.what());
		}

		std::array<vk::WriteDescriptorSet, 4> descriptorWrites{};

		descriptorWrites[0].dstSet = m_DescriptorSet;
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = vk::DescriptorType::eUniformBuffer;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &bufferInfo;

		// Albedo texture
		descriptorWrites[1].dstSet = m_DescriptorSet;
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = vk::DescriptorType::eCombinedImageSampler;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pBufferInfo = &bufferInfo;
		descriptorWrites[1].pImageInfo = &imageInfos[0];

		// Normal texture
		descriptorWrites[2].dstSet = m_DescriptorSet;
		descriptorWrites[2].dstBinding = 2;
		descriptorWrites[2].dstArrayElement = 0;
		descriptorWrites[2].descriptorType = vk::DescriptorType::eCombinedImageSampler;
		descriptorWrites[2].descriptorCount = 1;
		descriptorWrites[2].pBufferInfo = &bufferInfo;
		descriptorWrites[2].pImageInfo = &imageInfos[1];

		// Metallic Roughness texture
		descriptorWrites[3].dstSet = m_DescriptorSet;
		descriptorWrites[3].dstBinding = 3;
		descriptorWrites[3].dstArrayElement = 0;
		descriptorWrites[3].descriptorType = vk::DescriptorType::eCombinedImageSampler;
		descriptorWrites[3].descriptorCount = 1;
		descriptorWrites[3].pBufferInfo = &bufferInfo;
		descriptorWrites[3].pImageInfo = &imageInfos[2];

		VulkanRenderer::GetDevice().updateDescriptorSets(descriptorWrites, {});
	}
}
