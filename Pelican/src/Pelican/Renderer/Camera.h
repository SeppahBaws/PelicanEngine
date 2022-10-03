#pragma once
#include <glm/glm.hpp>

#include "Pelican/Core/Context.h"
#include "Pelican/Events/Event.h"
#include "Pelican/Events/KeyEvent.h"
#include "Pelican/Events/ApplicationEvent.h"

namespace Pelican
{
	enum class KeyCode;
	class Event;

	struct CameraPushConst
	{
		glm::vec3 eyePos;
	};

	class Camera
	{
	public:
		Camera(Context* pContext, f32 fov, f32 width, f32 height, f32 zNear, f32 zFar);

		void OnEvent(Event& e);
		void Update();

		[[nodiscard]] glm::mat4 GetView();
		[[nodiscard]] glm::mat4 GetProjection() const;
		[[nodiscard]] glm::vec3 GetPosition() const;

	private:
		bool OnResize(WindowResizeEvent& e);
		bool OnKeyPressed(KeyPressedEvent& e);

		// Pass 0 if you want to keep the original values
		void UpdateProjection(f32 fov, f32 width, f32 height, f32 zNear, f32 zFar);
		void UpdateViewMatrix();

	private:
		Context* m_pContext;

		glm::vec3 m_Position{};
		glm::vec3 m_Forward{};

		f32 m_Friction{ 0.05f };
		glm::vec3 m_Velocity{};

		glm::vec3 m_Rotation{};

		f32 m_MoveSpeed{ 40.0f };
		f32 m_LookSpeed{ 10.0f };
		f32 m_MoveSpeedSlow{ 10.0f };
		f32 m_MouseSpeed{ 5.0f };
		bool m_IsSlowMovement{ false };

		glm::mat4 m_View{};
		glm::mat4 m_Projection{};

		f32 m_Yaw;
		f32 m_Pitch;

		f32 m_Fov;
		f32 m_Width;
		f32 m_Height;
		f32 m_ZNear;
		f32 m_ZFar;
	};
}
