#pragma once
#include <glm/glm.hpp>

#include "Pelican/Events/Event.h"
#include "Pelican/Events/KeyEvent.h"
#include "Pelican/Events/ApplicationEvent.h"

namespace Pelican
{
	enum class KeyCode;
	class Event;

	class Camera
	{
	public:
		Camera(float fov, float width, float height, float zNear, float zFar);

		void OnEvent(Event& e);
		void Update();

		[[nodiscard]] glm::mat4 GetView();
		[[nodiscard]] glm::mat4 GetProjection() const;

	private:
		bool OnResize(WindowResizeEvent& e);
		bool OnKeyPressed(KeyPressedEvent& e);

		// Pass 0 if you want to keep the original values
		void UpdateProjection(float fov, float width, float height, float zNear, float zFar);
		void UpdateViewMatrix();

	private:
		glm::vec3 m_Position{};
		glm::vec3 m_Forward{};

		glm::vec3 m_Rotation{};

		float m_MoveSpeed{ 40.0f };
		float m_MoveSpeedSlow{ 10.0f };
		float m_MouseSpeed{ 5.0f };
		bool m_IsSlowMovement{ false };

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
