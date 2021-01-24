#pragma once

#pragma warning(push, 0) // Disable all warnings on external libraries
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#pragma warning(pop)

#include "Pelican/Renderer/Gltf/GltfModel.h"

namespace Pelican
{

	// TODO: add a ScriptComponent, which would be a base class and have these functions:
	// - OnCreate()
	// - OnUpdate()  <-+  main loop
	// - OnRender()  --+
	// - OnDestroy()
	//
	// Equivalent of BaseComponent. These components will allow the Sandbox app to script its own components.
	// look at https://github.com/TheCherno/Hazel/blob/master/Hazel/src/Hazel/Scene/ScriptableEntity.h

	struct TagComponent
	{
		std::string name;

		TagComponent() = default;
		explicit TagComponent(std::string tag)
			: name(std::move(tag)) {}
	};

	struct TransformComponent
	{
		glm::vec3 position;
		glm::vec3 rotation;
		glm::vec3 scale;

		TransformComponent() = default;

		[[nodiscard]] glm::mat4 GetTransform() const
		{
			return glm::translate(glm::mat4(1.0f), position)
				* glm::toMat4(glm::quat(rotation))
				* glm::scale(glm::mat4(1.0f), scale);
		}
	};

	struct ModelComponent
	{
		GltfModel* pModel;
	};
}
