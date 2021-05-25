#pragma once
#include <glm/vec4.hpp>
#include <glm/vec3.hpp>

namespace Pelican
{
	class VulkanTexture;

	// Wraps around a Gltf Material. More information:
	// https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#materials
	struct GltfMaterial final
	{
		glm::vec4 m_AlbedoColor;
		VulkanTexture* m_pAlbedoTexture;

		float m_MetallicFactor;
		float m_RoughnessFactor;
		VulkanTexture* m_pMetallicRoughnessTexture;

		// These are optional, they will be supplied by a default texture if not present in the Gltf Material.
		VulkanTexture* m_pNormalTexture;
		VulkanTexture* m_pAOTexture;
		VulkanTexture* m_pEmissiveTexture;
		glm::vec3 m_EmissiveFactor;
	};
}
