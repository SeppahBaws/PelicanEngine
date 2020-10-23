#pragma once

#include "Pelican/Renderer/Mesh.h"
#include "Pelican/Renderer/VulkanTexture.h"

namespace tinygltf
{
	class Model;
}

namespace Pelican
{
	struct GltfModelTextureBinding
	{
		Mesh mesh;
		VulkanTexture texture;
	};

	class GltfModel
	{
	public:
		GltfModel(const std::string& file);
		~GltfModel();

	private:
		void Initialize(const std::string& file);
		void Load(const tinygltf::Model& model);

	private:
		std::map<Mesh, VulkanTexture> m_Meshes;
	};
}
