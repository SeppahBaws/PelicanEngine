﻿#pragma once

#include "Pelican/Renderer/Camera.h"
#include "Pelican/Renderer/Camera.h"
#include "Pelican/Renderer/Camera.h"
#include "Pelican/Renderer/Camera.h"
#include "Pelican/Renderer/Camera.h"
#include "Pelican/Renderer/Camera.h"
#include "Pelican/Renderer/Mesh.h"
#include "Pelican/Renderer/VulkanTexture.h"

namespace tinygltf
{
	class Model;
}

namespace Pelican
{
	class Camera;

	struct GltfMaterial
	{
		int ColorTextureIdx;
	};

	class GltfModel
	{
	public:
		GltfModel(const std::string& file);
		~GltfModel();

		void Update(const glm::mat4& model, const glm::mat4& view, const glm::mat4& proj);
		void Draw();

		std::string GetAssetPath() const { return m_AssetPath; }

	private:
		void Initialize(const std::string& file);
		void Load(const tinygltf::Model& model);


		void CreateDescriptorPool();

		std::string GetAbsolutePath(const std::string& uri) const;

	private:
		std::vector<Mesh> m_Meshes;
		std::vector<GltfMaterial> m_Materials;
		std::vector<VulkanTexture*> m_pTextures;

		std::string m_AssetPath{};

		vk::DescriptorPool m_DescriptorPool{};
		VulkanTexture* m_pWhiteTexture{};
	};
}
