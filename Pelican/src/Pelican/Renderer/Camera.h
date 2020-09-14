#pragma once
#include <glm/glm.hpp>

namespace Pelican
{
	class Camera
	{
	public:
		Camera(float fov, float width, float height, float zNear, float zFar);

		void Update();

		glm::mat4 GetView();
		glm::mat4 GetProjection() const;

	private:
		void UpdateViewMatrix();

	private:
		glm::vec3 m_Position{};
		glm::vec3 m_Forward{};

		glm::vec3 m_Rotation{};

		float m_MoveSpeed{ 10.0f };
		float m_MouseSpeed{ 20.0f };

		glm::mat4 m_View{};
		glm::mat4 m_Projection{};

		float m_Yaw;
		float m_Pitch;
	};
}
