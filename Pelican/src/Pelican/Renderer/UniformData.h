#pragma once
#include <glm/glm.hpp>

namespace Pelican
{
#pragma warning (push)
#pragma warning (disable:4324) // padding-related stuff
	struct DirectionalLight
	{
		alignas(16) glm::vec3 direction;
		alignas(16) glm::vec3 lightColor;
		alignas(16) glm::vec3 ambientColor;
	};

	struct PointLight
	{
		alignas(16) glm::vec3 position;
		alignas(16) glm::vec3 diffuse;
	};
#pragma warning (pop)

	struct LightsData
	{
		DirectionalLight directionalLight;
		PointLight pointLight;
	};

	struct UniformBufferObject
	{
		alignas(16) glm::mat4 model;
		alignas(16) glm::mat4 view;
		alignas(16) glm::mat4 proj;
	};
}
