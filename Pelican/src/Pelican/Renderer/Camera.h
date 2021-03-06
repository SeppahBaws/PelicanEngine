﻿#pragma once
#include <glm/glm.hpp>

namespace Pelican
{
	class Camera
	{
	public:
		Camera(float fov, float width, float height, float zNear, float zFar);

		void Update();

		[[nodiscard]] glm::mat4 GetView();
		[[nodiscard]] glm::mat4 GetProjection() const;

		// Pass 0 if you want to keep the original values
		void UpdateProjection(float fov, float width, float height, float zNear, float zFar);

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

		float m_Fov;
		float m_Width;
		float m_Height;
		float m_ZNear;
		float m_ZFar;
	};
}
