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
#pragma warning (pop)

	struct UniformBufferObject
	{
		alignas(16) glm::mat4 model;
		alignas(16) glm::mat4 view;
		alignas(16) glm::mat4 proj;
	};
}
