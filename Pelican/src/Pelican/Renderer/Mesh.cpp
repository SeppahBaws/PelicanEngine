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
#pragma warning(pop)

#include "Model.h"
#include "Gltf/GltfMaterial.h"
#include "Pelican/Renderer/UniformData.h"
#include "Pelican/Core/Application.h"
#include "Pelican/Scene/Scene.h"

namespace Pelican
{
	Mesh::Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices, uint32_t materialIdx)
		: m_Vertices(std::move(vertices)), m_Indices(std::move(indices)), m_MaterialIdx(materialIdx)
	{
	}

	void Mesh::Cleanup()
	{
		delete m_pIndexBuffer;
		delete m_pVertexBuffer;
		delete m_pUniformBuffer;
		delete m_pLightBuffer;
	}

	void Mesh::SetupVerticesIndices(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
	{
		m_Vertices = vertices;
		m_Indices = indices;

		CreateBuffers();
		// CreateDescriptorSet();
	}

	void Mesh::Update(const glm::mat4& model, const glm::mat4& view, const glm::mat4& proj)
	{
		UniformBufferObject ubo{};
		ubo.model = model;
		ubo.view = view;
		ubo.proj = proj;

		void* data = m_pUniformBuffer->Map(sizeof(ubo));
			memcpy(data, &ubo, sizeof(ubo));
		m_pUniformBuffer->Unmap();

		LightsData lights;
		lights.directionalLight = Application::Get().GetScene()->GetDirectionalLight();
		lights.pointLight = Application::Get().GetScene()->GetPointLight();

		data = m_pLightBuffer->Map(sizeof(LightsData));
			memcpy(data, &lights, sizeof(lights));
		m_pLightBuffer->Unmap();
	}

	void Mesh::Draw(u32 frameIdx) const
	{
		vk::CommandBuffer commandBuffer = VulkanRenderer::GetCurrentBuffer();

		commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, VulkanRenderer::GetPipelineLayout(), 0, m_DescriptorSets[frameIdx], {});

		std::vector<vk::Buffer> vertexBuffers = { m_pVertexBuffer->GetBuffer() };
		std::vector<vk::DeviceSize> offsets = { 0 };
		commandBuffer.bindVertexBuffers(0, vertexBuffers, offsets);
		commandBuffer.bindIndexBuffer(m_pIndexBuffer->GetBuffer(), 0, vk::IndexType::eUint32);

		commandBuffer.drawIndexed(static_cast<uint32_t>(m_Indices.size()), 1, 0, 0, 0);
	}

	void Mesh::CreateBuffers()
	{
		// MVP Uniform buffer
		m_pUniformBuffer = new VulkanBuffer(
			sizeof(UniformBufferObject),
			vk::BufferUsageFlagBits::eUniformBuffer,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

		// Light Uniform buffer
		m_pLightBuffer = new VulkanBuffer(
			sizeof(LightsData),
			vk::BufferUsageFlagBits::eUniformBuffer,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

		// Vertex Buffer
		{
			const vk::DeviceSize bufferSize = sizeof(m_Vertices[0]) * m_Vertices.size();

			VulkanBuffer stagingBuffer{
				bufferSize,
				vk::BufferUsageFlagBits::eTransferSrc,
				vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
			};

			void* data = stagingBuffer.Map(bufferSize);
				memcpy(data, m_Vertices.data(), bufferSize);
			stagingBuffer.Unmap();

			m_pVertexBuffer = new VulkanBuffer{
				bufferSize,
				vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
				vk::MemoryPropertyFlagBits::eDeviceLocal
			};

			m_pVertexBuffer->CopyFromBuffer(stagingBuffer);
		}

		// Index Buffer
		{
			const vk::DeviceSize bufferSize = sizeof(m_Indices[0]) * m_Indices.size();

			VulkanBuffer stagingBuffer{
				bufferSize,
				vk::BufferUsageFlagBits::eTransferSrc,
				vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
			};

			void* data = stagingBuffer.Map(bufferSize);
				memcpy(data, m_Indices.data(), bufferSize);
			stagingBuffer.Unmap();

			m_pIndexBuffer = new VulkanBuffer{
				bufferSize,
				vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
				vk::MemoryPropertyFlagBits::eDeviceLocal
			};

			m_pIndexBuffer->CopyFromBuffer(stagingBuffer);
		}
	}

	// TODO: Descriptor Sets shouldn't be in the mesh.
	void Mesh::CreateDescriptorSet(const Model* pParent, const vk::DescriptorPool& pool)
	{
		vk::DescriptorBufferInfo mvpBufferInfo(m_pUniformBuffer->GetBuffer(), 0, sizeof(UniformBufferObject));
		vk::DescriptorBufferInfo lightBufferInfo(m_pLightBuffer->GetBuffer(), 0, sizeof(LightsData));

		const GltfMaterial& mat = pParent->GetMaterial(m_MaterialIdx);

		const std::array<vk::DescriptorImageInfo, static_cast<uint32_t>(TextureSlot::SLOT_COUNT)> imageInfos =
		{
			mat.m_pAlbedoTexture->GetDescriptorImageInfo(),
			mat.m_pNormalTexture->GetDescriptorImageInfo(),
			mat.m_pMetallicRoughnessTexture->GetDescriptorImageInfo(),
			mat.m_pAOTexture->GetDescriptorImageInfo(),
		};
		
		const vk::DescriptorImageInfo skyboxInfo = Application::Get().GetScene()->GetSkybox()->GetDescriptorImageInfo();
		const vk::DescriptorImageInfo radianceInfo = Application::Get().GetScene()->GetRadiance()->GetDescriptorImageInfo();
		const vk::DescriptorImageInfo irradianceInfo = Application::Get().GetScene()->GetIrradiance()->GetDescriptorImageInfo();

		vk::DescriptorSetAllocateInfo allocInfo = vk::DescriptorSetAllocateInfo()
			.setDescriptorPool(pool)
			.setSetLayouts(VulkanRenderer::GetDescriptorSetLayout());

		try
		{
			for (u32 i = 0; i < 3; i++)
			{
				std::vector<vk::DescriptorSet> sets = VulkanRenderer::GetDevice().allocateDescriptorSets(allocInfo);

				ASSERT_MSG(!sets.empty(), "No descriptors were allocated!");

				m_DescriptorSets[i] = sets[0];
			}
		}
		catch (vk::SystemError& e)
		{
			throw std::runtime_error("Failed to allocate descriptor sets: "s + e.what());
		}

		std::array<vk::WriteDescriptorSet, 9> descriptorWrites{};

		for (u32 i = 0; i < 3; i++)
		{
			// MVP Uniform buffer
			descriptorWrites[0] = vk::WriteDescriptorSet()
				.setDstSet(m_DescriptorSets[i])
				.setDstBinding(0)
				.setDstArrayElement(0)
				.setDescriptorType(vk::DescriptorType::eUniformBuffer)
				.setDescriptorCount(1)
				.setBufferInfo(mvpBufferInfo);
			// descriptorWrites[0].dstSet = m_DescriptorSet;
			// descriptorWrites[0].dstBinding = 0;
			// descriptorWrites[0].dstArrayElement = 0;
			// descriptorWrites[0].descriptorType = vk::DescriptorType::eUniformBuffer;
			// descriptorWrites[0].descriptorCount = 1;
			// descriptorWrites[0].pBufferInfo = &mvpBufferInfo;

			// Lights Uniform buffer
			descriptorWrites[1].dstSet = m_DescriptorSets[i];
			descriptorWrites[1].dstBinding = 1;
			descriptorWrites[1].dstArrayElement = 0;
			descriptorWrites[1].descriptorType = vk::DescriptorType::eUniformBuffer;
			descriptorWrites[1].descriptorCount = 1;
			descriptorWrites[1].pBufferInfo = &lightBufferInfo;

			// Albedo texture
			descriptorWrites[2].dstSet = m_DescriptorSets[i];
			descriptorWrites[2].dstBinding = 2;
			descriptorWrites[2].dstArrayElement = 0;
			descriptorWrites[2].descriptorType = vk::DescriptorType::eCombinedImageSampler;
			descriptorWrites[2].descriptorCount = 1;
			descriptorWrites[2].pImageInfo = &imageInfos[0];

			// Normal texture
			descriptorWrites[3].dstSet = m_DescriptorSets[i];
			descriptorWrites[3].dstBinding = 3;
			descriptorWrites[3].dstArrayElement = 0;
			descriptorWrites[3].descriptorType = vk::DescriptorType::eCombinedImageSampler;
			descriptorWrites[3].descriptorCount = 1;
			descriptorWrites[3].pImageInfo = &imageInfos[1];

			// Metallic Roughness texture
			descriptorWrites[4].dstSet = m_DescriptorSets[i];
			descriptorWrites[4].dstBinding = 4;
			descriptorWrites[4].dstArrayElement = 0;
			descriptorWrites[4].descriptorType = vk::DescriptorType::eCombinedImageSampler;
			descriptorWrites[4].descriptorCount = 1;
			descriptorWrites[4].pImageInfo = &imageInfos[2];

			// AO texture
			descriptorWrites[5].dstSet = m_DescriptorSets[i];
			descriptorWrites[5].dstBinding = 5;
			descriptorWrites[5].dstArrayElement = 0;
			descriptorWrites[5].descriptorType = vk::DescriptorType::eCombinedImageSampler;
			descriptorWrites[5].descriptorCount = 1;
			descriptorWrites[5].pImageInfo = &imageInfos[3];

			descriptorWrites[6] = vk::WriteDescriptorSet()
				.setDstSet(m_DescriptorSets[i])
				.setDstBinding(6)
				.setDstArrayElement(0)
				.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
				.setDescriptorCount(1)
				.setPImageInfo(&skyboxInfo);

			descriptorWrites[7] = vk::WriteDescriptorSet()
				.setDstSet(m_DescriptorSets[i])
				.setDstBinding(7)
				.setDstArrayElement(0)
				.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
				.setDescriptorCount(1)
				.setPImageInfo(&radianceInfo);

			descriptorWrites[8] = vk::WriteDescriptorSet()
				.setDstSet(m_DescriptorSets[i])
				.setDstBinding(8)
				.setDstArrayElement(0)
				.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
				.setDescriptorCount(1)
				.setPImageInfo(&irradianceInfo);

			VulkanRenderer::GetDevice().updateDescriptorSets(descriptorWrites, {});
		}
	}
}
