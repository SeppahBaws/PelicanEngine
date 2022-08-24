#include "PelicanPCH.h"
#include "Model.h"

#include "Pelican/Assets/AssetManager.h"

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

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

namespace Pelican
{
	Model::Model(const std::string& file)
		: m_AssetPath(file)
	{
		Initialize();
	}

	Model::~Model()
	{
		// Cleanup the material textures
		for (GltfMaterial& mat : m_Materials)
		{
			AssetManager::GetInstance().UnloadTexture(mat.m_pAlbedoTexture);
			AssetManager::GetInstance().UnloadTexture(mat.m_pMetallicRoughnessTexture);
			AssetManager::GetInstance().UnloadTexture(mat.m_pNormalTexture);
			AssetManager::GetInstance().UnloadTexture(mat.m_pAOTexture);
			AssetManager::GetInstance().UnloadTexture(mat.m_pEmissiveTexture);
		}

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

	void Model::UpdateDrawData(const glm::mat4& model, const glm::mat4& view, const glm::mat4& proj)
	{
		for (size_t i = 0; i < m_Meshes.size(); i++)
		{
			m_Meshes[i].Update(model, view, proj);
		}
	}

	void Model::Draw(u32 frameIdx)
	{
		for (size_t i = 0; i < m_Meshes.size(); i++)
		{
			m_Meshes[i].Draw(frameIdx);
		}
	}

	void Model::Initialize()
	{
		Assimp::Importer importer;
		const aiScene* pScene = importer.ReadFile(m_AssetPath, aiProcessPreset_TargetRealtime_Quality | aiProcess_FlipUVs);

		if (!pScene || pScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !pScene->mRootNode)
		{
			Logger::LogError("Assimp import error: %s", importer.GetErrorString());
			return;
		}

		// Materials
		for (uint32_t i = 0; i < pScene->mNumMaterials; i++)
		{
			aiMaterial* pMaterial = pScene->mMaterials[i];
			GltfMaterial mat{};

			aiString texturePath;
			aiColor4D textureColor;
			ai_real textureFloat;

			if (AI_SUCCESS == aiGetMaterialColor(pMaterial, AI_MATKEY_COLOR_DIFFUSE, &textureColor))
				mat.m_AlbedoColor = glm::vec4(textureColor.r, textureColor.g, textureColor.b, textureColor.a);
			if (AI_SUCCESS == pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath))
				mat.m_pAlbedoTexture = AssetManager::GetInstance().LoadTexture(GetAbsolutePath(texturePath.C_Str()));
			else
				mat.m_pAlbedoTexture = AssetManager::GetInstance().LoadTexture("res/textures/default-white.png");

			if (AI_SUCCESS == aiGetMaterialFloat(pMaterial, AI_MATKEY_METALLIC_FACTOR, &textureFloat))
				mat.m_MetallicFactor = textureFloat;
			if (AI_SUCCESS == aiGetMaterialFloat(pMaterial, AI_MATKEY_ROUGHNESS_FACTOR, &textureFloat))
				mat.m_RoughnessFactor = textureFloat;
			if (AI_SUCCESS == pMaterial->GetTexture(aiTextureType_METALNESS, 0, &texturePath))
				mat.m_pMetallicRoughnessTexture = AssetManager::GetInstance().LoadTexture(GetAbsolutePath(texturePath.C_Str()));
			else
				mat.m_pMetallicRoughnessTexture = AssetManager::GetInstance().LoadTexture("res/textures/default-white.png");

			if (AI_SUCCESS == pMaterial->GetTexture(aiTextureType_NORMALS, 0, &texturePath))
				mat.m_pNormalTexture = AssetManager::GetInstance().LoadTexture(GetAbsolutePath(texturePath.C_Str()));
			else
				mat.m_pNormalTexture = AssetManager::GetInstance().LoadTexture("res/textures/default-normal.png");

			if (AI_SUCCESS == pMaterial->GetTexture(aiTextureType_AMBIENT_OCCLUSION, 0, &texturePath))
				mat.m_pAOTexture = AssetManager::GetInstance().LoadTexture(GetAbsolutePath(texturePath.C_Str()));
			else
				mat.m_pAOTexture = AssetManager::GetInstance().LoadTexture("res/textures/default-white.png");

			if (AI_SUCCESS == aiGetMaterialColor(pMaterial, AI_MATKEY_EMISSIVE_INTENSITY, &textureColor))
				mat.m_EmissiveFactor = glm::vec3(textureColor.r, textureColor.g, textureColor.b);
			if (AI_SUCCESS == pMaterial->GetTexture(aiTextureType_EMISSIVE, 0, &texturePath))
				mat.m_pEmissiveTexture = AssetManager::GetInstance().LoadTexture(GetAbsolutePath(texturePath.C_Str()));
			else
				mat.m_pEmissiveTexture = AssetManager::GetInstance().LoadTexture("res/textures/default-white.png");

			m_Materials.push_back(mat);
		}

		ProcessNode(pScene->mRootNode, pScene);

		CreateDescriptorPool();
	}

	void Model::ProcessNode(aiNode* pNode, const aiScene* pScene)
	{
		// Process all the node's meshes (if any)
		for (unsigned int i = 0; i < pNode->mNumMeshes; i++)
		{
			aiMesh* pMesh = pScene->mMeshes[pNode->mMeshes[i]];
			m_Meshes.push_back(ProcessMesh(pMesh));
		}

		// Then do the same for each of its children
		for (unsigned int i = 0; i < pNode->mNumChildren; i++)
		{
			ProcessNode(pNode->mChildren[i], pScene);
		}
	}

	Mesh Model::ProcessMesh(aiMesh* pMesh)
	{
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		// Vertices
		for (uint32_t i = 0; i < pMesh->mNumVertices; i++)
		{
			Vertex vertex{};

			vertex.pos = glm::vec3(0);
			vertex.normal = glm::vec3(0);
			vertex.tangent = glm::vec3(0, 0, 1);
			
			if (pMesh->mVertices)
			{
				vertex.pos = glm::vec3(pMesh->mVertices[i].x, pMesh->mVertices[i].y, pMesh->mVertices[i].z);
			}

			if (pMesh->mNormals)
			{
				vertex.normal = glm::vec3(pMesh->mNormals[i].x, pMesh->mNormals[i].y, pMesh->mNormals[i].z);
			}

			if (pMesh->mTangents)
			{
				vertex.tangent = glm::vec3(pMesh->mTangents[i].x, pMesh->mTangents[i].y, pMesh->mTangents[i].z);
			}

			if (pMesh->mTextureCoords[0])
			{
				vertex.texCoord = glm::vec2(pMesh->mTextureCoords[0][i].x, pMesh->mTextureCoords[0][i].y);
			}
			else
			{
				Logger::LogWarning("Mesh: UV coordinates not found! Default to (0, 0)");
				vertex.texCoord = glm::vec2(0);
			}

			vertices.push_back(vertex);
		}

		// Indices
		for (uint32_t i = 0; i < pMesh->mNumFaces; i++)
		{
			aiFace face = pMesh->mFaces[i];
			for (uint32_t j = 0; j < face.mNumIndices; j++)
			{
				indices.push_back(face.mIndices[j]);
			}
		}

		Mesh mesh{ vertices, indices, pMesh->mMaterialIndex };
		mesh.CreateBuffers();
		return mesh;
	}

	void Model::CreateDescriptorPool()
	{
		vk::DescriptorPoolSize poolSize(vk::DescriptorType::eUniformBuffer, static_cast<uint32_t>(m_Meshes.size()) * 3);

		vk::DescriptorPoolCreateInfo createInfo = vk::DescriptorPoolCreateInfo()
			.setPoolSizes(poolSize)
			.setMaxSets(static_cast<uint32_t>(m_Meshes.size()) * 3);

		try
		{
			m_DescriptorPool = VulkanRenderer::GetDevice().createDescriptorPool(createInfo);
		}
		catch (vk::SystemError& e)
		{
			throw std::runtime_error("Failed to create descriptor pool: "s + e.what());
		}

		for (size_t i = 0; i < m_Meshes.size(); i++)
		{
			m_Meshes[i].CreateDescriptorSet(this, m_DescriptorPool);
		}
	}

	std::string Model::GetAbsolutePath(const std::string& uri) const
	{
		std::string absolutePath = m_AssetPath;
		absolutePath = absolutePath.substr(0, m_AssetPath.find_last_of('/') + 1);
		absolutePath += uri;
		return absolutePath;
	}
}
