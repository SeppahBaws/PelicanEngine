#include "PelicanPCH.h"
#include "Mesh.h"

#include "VulkanHelpers.h"
#include "VulkanRenderer.h"

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
		: m_Vertices(), m_Indices()
	{
	}

	Mesh::Mesh(const std::string& filename)
	{
		tinygltf::Model model;
		tinygltf::TinyGLTF loader;
		std::string err;
		std::string warn;
	
		bool res = loader.LoadASCIIFromFile(&model, &err, &warn, filename);
		if (!warn.empty())
		{
			Logger::LogWarning(warn);
		}
	
		if (!err.empty())
		{
			Logger::LogError(err);
		}
	
		if (!res)
			Logger::LogWarning("Failed to load glTF: %s", filename.c_str());
		else
			Logger::LogDebug("Loaded glTF: %s", filename.c_str());

	
		tinygltf::Mesh mesh = model.meshes[0];
		for (size_t i = 0; i < mesh.primitives.size(); i++)
		{
			tinygltf::Primitive primitive = mesh.primitives[i];

			// const tinygltf::Accessor& posAccessor = model.accessors[primitive.attributes["POSITION"]];
			// const tinygltf::BufferView& posBufView = model.bufferViews[posAccessor.bufferView];
			// const tinygltf::Buffer& posBuf = model.buffers[posBufView.buffer];
			
			// const float* positions = reinterpret_cast<const float*>(&posBuf.data[posBufView.byteOffset + posAccessor.byteOffset]);
			// // posAccessor.ByteStride(posBufView) ? (posAccessor.ByteStride(posBufView) / sizeof(float) : tinygltf::)
			// int posByteStride = posAccessor.ByteStride(posBufView);
			// for (size_t v = 0; v < posAccessor.count; v++)
			// {
			// 	// Vertex vert = {
			// 	// 	{
			// 	// 		positions[i * 3 + 0],
			// 	// 		positions[i * 3 + 1],
			// 	// 		positions[i * 3 + 2],
			// 	// 	},
			// 	// 	{0.5f, 0.5f, 0.5f},
			// 	// 	{1.0f, 1.0f}
			// 	// };
			//
			// 	Vertex vert{};
			// 	vert.pos = glm::make_vec3(&positions[i * posByteStride]);
			// 	m_Vertices.push_back(vert);
			// }

			// if (primitive.indices <= 0)
			// {
			// 	Logger::LogError("No vertex indices were found!");
			// }
			//
			// // const tinygltf::Accessor& indAccessor = model.accessors[primitive.indices];
			// // const tinygltf::BufferView& indBufView = model.bufferViews[indAccessor.bufferView];
			// // const tinygltf::Buffer& indBuf = model.buffers[indBufView.buffer];
			// //
			// // const int* indices = reinterpret_cast<const int*>(&indBuf.data[indBufView.byteOffset + indAccessor.byteOffset]);
			// // for (size_t i = 0; i < indAccessor.count; i++)
			// // {
			// // 	m_Indices.push_back(static_cast<uint16_t>(indices[i * 2]));
			// // }
			//
			// const tinygltf::Accessor& indAccessor = model.accessors[primitive.indices];
			// const tinygltf::BufferView& indBufView = model.bufferViews[indAccessor.bufferView];
			// const tinygltf::Buffer& indBuf = model.buffers[indBufView.buffer];
			//
			// const void* dataPtr = &(indBuf.data[indAccessor.byteOffset + indBufView.byteOffset]);
			//
			// switch (indAccessor.componentType)
			// {
			// case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT:
			// {
			// 	const uint32_t* buf = static_cast<const uint32_t*>(dataPtr);
			// 	for (size_t index = 0; index < indAccessor.count; index++)
			// 	{
			// 		m_Indices.push_back(buf[index] /*+ vertexStart*/);
			// 	}
			// }
			// break;
			//
			// case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT:
			// {
			// 	const uint16_t* buf = static_cast<const uint16_t*>(dataPtr);
			// 	for (size_t index = 0; index < indAccessor.count; index++)
			// 	{
			// 		m_Indices.push_back(buf[index] /*+ vertexStart*/);
			// 	}
			// }
			// break;
			//
			// case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE:
			// {
			// 	const uint8_t* buf = static_cast<const uint8_t*>(dataPtr);
			// 	for (size_t index = 0; index < indAccessor.count; index++)
			// 	{
			// 		m_Indices.push_back(buf[index] /*+ vertexStart*/);
			// 	}
			// }
			// break;
			// }
	
			// uint32_t indexStart = static_cast<uint32_t>(m_Indices.size());
			uint16_t vertexStart = static_cast<uint16_t>(m_Vertices.size());
			uint32_t indexCount = 0;
			uint32_t vertexCount = 0;
			glm::vec3 posMin{};
			glm::vec3 posMax{};
			// bool hasSkin = false;
			bool hasIndices = primitive.indices > -1;
			// Vertices
			{
				const float* bufferPos = nullptr;
				// const float* bufferNormals = nullptr;
				const float* bufferTexCoordSet0 = nullptr;
				// const float* bufferTexCoordSet1 = nullptr;
				// const uint16_t* bufferJoints = nullptr;
				// const float* bufferWeights = nullptr;
			
				int posByteStride{};
				// int normByteStride;
				int uv0ByteStride{};
				// int uv1ByteStride;
				// int jointByteStride;
				// int weightByteStride;
			
				// Position attribute is required
				assert(primitive.attributes.find("POSITION") != primitive.attributes.end());
			
				const tinygltf::Accessor& posAccessor = model.accessors[primitive.attributes.find("POSITION")->second];
				const tinygltf::BufferView& posView = model.bufferViews[posAccessor.bufferView];
				bufferPos = reinterpret_cast<const float*>(&(model.buffers[posView.buffer].data[posAccessor.byteOffset + posView.byteOffset]));
				posMin = glm::vec3(posAccessor.minValues[0], posAccessor.minValues[1], posAccessor.minValues[2]);
				posMax = glm::vec3(posAccessor.maxValues[0], posAccessor.maxValues[1], posAccessor.maxValues[2]);
				vertexCount = static_cast<uint32_t>(posAccessor.count);
				posByteStride = posAccessor.ByteStride(posView) ? (posAccessor.ByteStride(posView) / sizeof(float)) : tinygltf::GetTypeSizeInBytes(TINYGLTF_TYPE_VEC3);
			
				// if (primitive.attributes.find("NORMAL") != primitive.attributes.end()) {
				// 	const tinygltf::Accessor& normAccessor = model.accessors[primitive.attributes.find("NORMAL")->second];
				// 	const tinygltf::BufferView& normView = model.bufferViews[normAccessor.bufferView];
				// 	bufferNormals = reinterpret_cast<const float*>(&(model.buffers[normView.buffer].data[normAccessor.byteOffset + normView.byteOffset]));
				// 	normByteStride = normAccessor.ByteStride(normView) ? (normAccessor.ByteStride(normView) / sizeof(float)) : tinygltf::GetTypeSizeInBytes(TINYGLTF_TYPE_VEC3);
				// }
				//
				if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end()) {
					const tinygltf::Accessor& uvAccessor = model.accessors[primitive.attributes.find("TEXCOORD_0")->second];
					const tinygltf::BufferView& uvView = model.bufferViews[uvAccessor.bufferView];
					bufferTexCoordSet0 = reinterpret_cast<const float*>(&(model.buffers[uvView.buffer].data[uvAccessor.byteOffset + uvView.byteOffset]));
					uv0ByteStride = uvAccessor.ByteStride(uvView) ? (uvAccessor.ByteStride(uvView) / sizeof(float)) : tinygltf::GetTypeSizeInBytes(TINYGLTF_TYPE_VEC2);
				}
				// if (primitive.attributes.find("TEXCOORD_1") != primitive.attributes.end()) {
				// 	const tinygltf::Accessor& uvAccessor = model.accessors[primitive.attributes.find("TEXCOORD_1")->second];
				// 	const tinygltf::BufferView& uvView = model.bufferViews[uvAccessor.bufferView];
				// 	bufferTexCoordSet1 = reinterpret_cast<const float*>(&(model.buffers[uvView.buffer].data[uvAccessor.byteOffset + uvView.byteOffset]));
				// 	uv1ByteStride = uvAccessor.ByteStride(uvView) ? (uvAccessor.ByteStride(uvView) / sizeof(float)) : tinygltf::GetTypeSizeInBytes(TINYGLTF_TYPE_VEC2);
				// }
				//
				// // Skinning
				// // Joints
				// if (primitive.attributes.find("JOINTS_0") != primitive.attributes.end()) {
				// 	const tinygltf::Accessor& jointAccessor = model.accessors[primitive.attributes.find("JOINTS_0")->second];
				// 	const tinygltf::BufferView& jointView = model.bufferViews[jointAccessor.bufferView];
				// 	bufferJoints = reinterpret_cast<const uint16_t*>(&(model.buffers[jointView.buffer].data[jointAccessor.byteOffset + jointView.byteOffset]));
				// 	jointByteStride = jointAccessor.ByteStride(jointView) ? (jointAccessor.ByteStride(jointView) / sizeof(bufferJoints[0])) : tinygltf::GetTypeSizeInBytes(TINYGLTF_TYPE_VEC4);
				// }
				//
				// if (primitive.attributes.find("WEIGHTS_0") != primitive.attributes.end()) {
				// 	const tinygltf::Accessor& weightAccessor = model.accessors[primitive.attributes.find("WEIGHTS_0")->second];
				// 	const tinygltf::BufferView& weightView = model.bufferViews[weightAccessor.bufferView];
				// 	bufferWeights = reinterpret_cast<const float*>(&(model.buffers[weightView.buffer].data[weightAccessor.byteOffset + weightView.byteOffset]));
				// 	weightByteStride = weightAccessor.ByteStride(weightView) ? (weightAccessor.ByteStride(weightView) / sizeof(float)) : tinygltf::GetTypeSizeInBytes(TINYGLTF_TYPE_VEC4);
				// }
				//
				// hasSkin = (bufferJoints && bufferWeights);
			
				for (size_t v = 0; v < posAccessor.count; v++) {
					Vertex vert{};
					vert.pos = glm::make_vec3(&bufferPos[v * posByteStride]);
					// vert.normal = glm::normalize(glm::vec3(bufferNormals ? glm::make_vec3(&bufferNormals[v * normByteStride]) : glm::vec3(0.0f)));
					vert.texCoord = bufferTexCoordSet0 ? glm::make_vec2(&bufferTexCoordSet0[v * uv0ByteStride]) : glm::vec3(0.0f);
					// vert.uv1 = bufferTexCoordSet1 ? glm::make_vec2(&bufferTexCoordSet1[v * uv1ByteStride]) : glm::vec3(0.0f);
					//
					// vert.joint0 = hasSkin ? glm::vec4(glm::make_vec4(&bufferJoints[v * jointByteStride])) : glm::vec4(0.0f);
					// vert.weight0 = hasSkin ? glm::make_vec4(&bufferWeights[v * weightByteStride]) : glm::vec4(0.0f);
					// // Fix for all zero weights
					// if (glm::length(vert.weight0) == 0.0f) {
					// 	vert.weight0 = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
					// }
					m_Vertices.push_back(vert);
				}
			}

			// m_Vertices = {
			// 	Vertex{
			// 		{ -1.0f, 0.0f, -1.0f },
			// 		{ 0.0f, 0.0, 0.0f },
			// 		{ 0.0f, 0.0f }
			// 	},
			// 	Vertex{
			// 		{ 1.0f, 0.0f, -1.0f },
			// 		{ 0.0f, 0.0, 0.0f },
			// 		{ 0.0f, 0.0f }
			// 	},
			// 	Vertex{
			// 		{ 0.0f, 0.0f, 1.0f },
			// 		{ 0.0f, 0.0, 0.0f },
			// 		{ 0.0f, 0.0f }
			// 	}
			// };

			// Indices
			if (hasIndices)
			{
				const tinygltf::Accessor& accessor = model.accessors[primitive.indices > -1 ? primitive.indices : 0];
				const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
				const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];
	
				indexCount = static_cast<uint32_t>(accessor.count);
				const void* dataPtr = &(buffer.data[accessor.byteOffset + bufferView.byteOffset]);
	
				// switch (accessor.componentType) {
				// case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
					const uint32_t* buf = static_cast<const uint32_t*>(dataPtr);
					for (size_t index = 0; index < accessor.count; index++) {
						m_Indices.push_back(static_cast<uint16_t>(buf[index] + vertexStart));
					}
				// 	break;
				// }
				// case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
				// 	const uint16_t* buf = static_cast<const uint16_t*>(dataPtr);
				// 	for (size_t index = 0; index < accessor.count; index++) {
				// 		m_Indices.push_back(buf[index] + vertexStart);
				// 	}
				// 	break;
				// }
				// case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
				// 	const uint8_t* buf = static_cast<const uint8_t*>(dataPtr);
				// 	for (size_t index = 0; index < accessor.count; index++) {
				// 		m_Indices.push_back(buf[index] + vertexStart);
				// 	}
				// 	break;
				// }
				// default:
				// 	std::cerr << "Index component type " << accessor.componentType << " not supported!" << std::endl;
				// 	return;
				// }
			}
			// Primitive* newPrimitive = new Primitive(indexStart, indexCount, vertexCount, primitive.material > -1 ? materials[primitive.material] : materials.back());
			// newPrimitive->setBoundingBox(posMin, posMax);
			// newMesh->primitives.push_back(newPrimitive);
		}
	}

	Mesh::Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices)
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

	void Mesh::SetupVerticesIndices(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
	{
		m_Vertices = vertices;
		m_Indices = indices;

		CreateBuffers();
	}

	void Mesh::Draw()
	{
		VkCommandBuffer commandBuffer = VulkanRenderer::GetCurrentBuffer();

		VkBuffer vertexBuffers[] = { m_VkVertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(commandBuffer, m_VkIndexBuffer, 0, VK_INDEX_TYPE_UINT32);

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
