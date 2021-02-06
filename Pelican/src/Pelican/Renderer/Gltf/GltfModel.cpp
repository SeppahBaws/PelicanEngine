#include "PelicanPCH.h"
#include "GltfModel.h"

#include "Pelican/Renderer/Camera.h"
#include "Pelican/Renderer/Mesh.h"
#include "Pelican/Renderer/VulkanHelpers.h"
#include "Pelican/Renderer/VulkanTexture.h"
#include "Pelican/Renderer/VulkanRenderer.h"

#include <logtools.h>

#pragma warning(push)
#pragma warning(disable:4996)
#pragma warning(disable:4840)
#pragma warning(disable:4201)
// #define TINYGLTF_IMPLEMENTATION
// #define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_NOEXCEPTION
#define JSON_NOEXCEPTION
#include <glm/vec3.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "tiny_gltf.h"
#pragma warning(pop)

namespace Pelican
{
	GltfModel::GltfModel(const std::string& file)
		: m_AssetPath(file)
	{
		Initialize(file);
	}

	GltfModel::~GltfModel()
	{
		vkDestroyDescriptorPool(VulkanRenderer::GetDevice(), m_DescriptorPool, nullptr);

		for (size_t i = 0; i < m_Meshes.size(); i++)
		{
			m_Meshes[i].Cleanup();
		}

		for (size_t i = 0; i < m_pTextures.size(); i++)
		{
			delete m_pTextures[i];
			m_pTextures[i] = nullptr;
		}
		m_pTextures.clear();
		delete m_pWhiteTexture;
	}

	void GltfModel::Update(const glm::mat4& model, const glm::mat4& view, const glm::mat4& proj)
	{
		for (size_t i = 0; i < m_Meshes.size(); i++)
		{
			m_Meshes[i].Update(model, view, proj);
		}
	}

	void GltfModel::Draw()
	{
		for (size_t i = 0; i < m_Meshes.size(); i++)
		{
			m_Meshes[i].Draw();
		}
	}

	void GltfModel::Initialize(const std::string& file)
	{
		tinygltf::Model model;
		tinygltf::TinyGLTF loader;
		std::string err;
		std::string warn;

		bool res = loader.LoadASCIIFromFile(&model, &err, &warn, file);
		if (!warn.empty())
		{
			Logger::LogWarning(warn);
		}

		if (!err.empty())
		{
			Logger::LogError(err);
		}

		if (!res)
			Logger::LogWarning("Failed to load glTF: %s", file.c_str());
		else
			Logger::LogDebug("Loaded glTF: %s", file.c_str());

		Load(model);
	}

	static inline int32_t GetTypeSizeInBytes(uint32_t type)
	{
		switch (type)
		{
		case TINYGLTF_TYPE_SCALAR:
			return 1;
		case TINYGLTF_TYPE_VEC2:
			return 2;
		case TINYGLTF_TYPE_VEC3:
			return 3;
		case TINYGLTF_TYPE_VEC4:
			return 4;
		case TINYGLTF_TYPE_MAT2:
			return 4;
		case TINYGLTF_TYPE_MAT3:
			return 9;
		case TINYGLTF_TYPE_MAT4:
			return 16;
		default:
			// Unknown component type
			return -1;
		}
	}

	void GltfModel::Load(const tinygltf::Model& model)
	{
		Logger::LogInfo("Model copyright: %s", model.asset.copyright.c_str());
		Logger::LogInfo("Model version: %s", model.asset.version.c_str());
		Logger::LogInfo("Model generator: %s", model.asset.generator.c_str());
		Logger::LogInfo("Model extra info: %s", model.asset.extras_json_string.c_str());
		for (tinygltf::Node node : model.nodes)
		{
			Logger::LogInfo("Node info:");
			Logger::LogInfo("  name: %s", node.name.c_str());
			Logger::LogInfo("  child count: %d", node.children.size());

			if (node.mesh > -1)
			{
				tinygltf::Mesh gltfMesh = model.meshes[node.mesh];
				tinygltf::Primitive primitive = gltfMesh.primitives[0];
			
				std::vector<Vertex> vertices;
				std::vector<uint32_t> indices;
			
				uint16_t vertexStart = static_cast<uint16_t>(vertices.size());
				uint32_t indexCount = 0;
				uint32_t vertexCount = 0;
				// glm::vec3 posMin{};
				// glm::vec3 posMax{};
				bool hasIndices = primitive.indices > -1;
			
				// Vertices
				{
					const float* bufferPos = nullptr;
					const float* bufferNorm = nullptr;
					const float* bufferTexCoordSet0 = nullptr;
			
					int posByteStride{};
					int normByteStride{};
					int uv0ByteStride{};
			
					const tinygltf::Accessor& posAccessor = model.accessors[primitive.attributes.find("POSITION")->second];
					const tinygltf::BufferView& posView = model.bufferViews[posAccessor.bufferView];
					bufferPos = reinterpret_cast<const float*>(&(model.buffers[posView.buffer].data[posAccessor.byteOffset + posView.byteOffset]));
					// posMin = glm::vec3(posAccessor.minValues[0], posAccessor.minValues[1], posAccessor.minValues[2]);
					// posMax = glm::vec3(posAccessor.maxValues[0], posAccessor.maxValues[1], posAccessor.maxValues[2]);

					vertexCount = static_cast<uint32_t>(posAccessor.count);
					posByteStride = posAccessor.ByteStride(posView) ? (posAccessor.ByteStride(posView) / sizeof(float)) : GetTypeSizeInBytes(TINYGLTF_TYPE_VEC3);

					if (primitive.attributes.find("NORMAL") != primitive.attributes.end())
					{
						const tinygltf::Accessor& normAccessor = model.accessors[primitive.attributes.find("NORMAL")->second];
						const tinygltf::BufferView& normView = model.bufferViews[normAccessor.bufferView];
						bufferNorm = reinterpret_cast<const float*>(&(model.buffers[normView.buffer].data[normAccessor.byteOffset + normView.byteOffset]));
						normByteStride = normAccessor.ByteStride(normView) ? (normAccessor.ByteStride(normView) / sizeof(float)) : GetTypeSizeInBytes(TINYGLTF_TYPE_VEC3);
					}

					if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end())
					{
						const tinygltf::Accessor& uvAccessor = model.accessors[primitive.attributes.find("TEXCOORD_0")->second];
						const tinygltf::BufferView& uvView = model.bufferViews[uvAccessor.bufferView];
						bufferTexCoordSet0 = reinterpret_cast<const float*>(&(model.buffers[uvView.buffer].data[uvAccessor.byteOffset + uvView.byteOffset]));
						uv0ByteStride = uvAccessor.ByteStride(uvView) ? (uvAccessor.ByteStride(uvView) / sizeof(float)) : GetTypeSizeInBytes(TINYGLTF_TYPE_VEC2);
					}
			
					for (size_t v = 0; v < posAccessor.count; v++)
					{
						Vertex vert{};
						vert.pos = glm::make_vec3(&bufferPos[v * posByteStride]);
						vert.normal = bufferNorm ? glm::make_vec3(&bufferNorm[v * normByteStride]) : glm::vec3(0.0f);
						vert.texCoord = bufferTexCoordSet0 ? glm::make_vec2(&bufferTexCoordSet0[v * uv0ByteStride]) : glm::vec3(0.0f);
						vertices.push_back(vert);
					}
				}
			
				// Indices
				if (hasIndices)
				{
					const tinygltf::Accessor& accessor = model.accessors[primitive.indices > -1 ? primitive.indices : 0];
					const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
					const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];
			
					indexCount = static_cast<uint32_t>(accessor.count);
					const void* dataPtr = &(buffer.data[accessor.byteOffset + bufferView.byteOffset]);
			
					const uint32_t* buf = static_cast<const uint32_t*>(dataPtr);
					for (size_t index = 0; index < accessor.count; index++) {
						indices.push_back(static_cast<uint16_t>(buf[index] + vertexStart));
					}
				}

				Mesh mesh{ vertices, indices };

				if (!model.materials.empty())
				{
					tinygltf::Material material = model.materials[primitive.material];

					if (material.pbrMetallicRoughness.baseColorTexture.index > -1)
					{
						tinygltf::Image albedoImage = model.images[material.pbrMetallicRoughness.baseColorTexture.index];
						mesh.SetupTexture(TextureSlot::ALBEDO, GetAbsolutePath(albedoImage.uri));
					}

					if (material.normalTexture.index > -1)
					{
						tinygltf::Image normalImage = model.images[material.normalTexture.index];
						mesh.SetupTexture(TextureSlot::NORMAL, GetAbsolutePath(normalImage.uri));
					}

					if (material.pbrMetallicRoughness.metallicRoughnessTexture.index > -1)
					{
						tinygltf::Image metallicRoughnessImage = model.images[material.pbrMetallicRoughness.metallicRoughnessTexture.index];
						mesh.SetupTexture(TextureSlot::METALLIC_ROUGHNESS, GetAbsolutePath(metallicRoughnessImage.uri));
					}
				}

				mesh.CreateBuffers();
				m_Meshes.push_back(mesh);
			}
		}

		CreateDescriptorPool();
	}

	void GltfModel::CreateDescriptorPool()
	{
		VkDescriptorPoolSize poolSize{};
		poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSize.descriptorCount = static_cast<uint32_t>(m_Meshes.size());

		VkDescriptorPoolCreateInfo createInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
		createInfo.poolSizeCount = 1;
		createInfo.pPoolSizes = &poolSize;
		createInfo.maxSets = static_cast<uint32_t>(m_Meshes.size());

		VK_CHECK(vkCreateDescriptorPool(VulkanRenderer::GetDevice(), &createInfo, nullptr, &m_DescriptorPool));

		for (size_t i = 0; i < m_Meshes.size(); i++)
		{
			m_Meshes[i].CreateDescriptorSet(m_DescriptorPool);
		}
	}

	std::string GltfModel::GetAbsolutePath(const std::string& uri) const
	{
		std::string absolutePath = m_AssetPath;
		absolutePath = absolutePath.substr(0, m_AssetPath.find_last_of('/') + 1);
		absolutePath += uri;
		return absolutePath;
	}
}
