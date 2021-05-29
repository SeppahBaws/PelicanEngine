#include "PelicanPCH.h"
#include "Camera.h"

#include <logtools.h>

#include "Pelican/Core/Time.h"

#include "Pelican/Events/Event.h"
#include "Pelican/Events/KeyEvent.h"

#include "Pelican/Input/Input.h"

#include <glm/gtc/matrix_transform.hpp>

#include "Pelican/Events/ApplicationEvent.h"

namespace Pelican
{
	Camera::Camera(float fov, float width, float height, float zNear, float zFar)
		: m_Position(-20.0f, 20.0f, 20.0f)
		, m_Forward(0.0f, 0.0f, 1.0f)
		, m_Yaw(0.0f)
		, m_Pitch(0.0f)
	{
		UpdateProjection(fov, width, height, zNear, zFar);
	}

	void Camera::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(Camera::OnResize));
		dispatcher.Dispatch<KeyPressedEvent>(BIND_EVENT_FN(Camera::OnKeyPressed));
	}

	void Camera::Update()
	{
		glm::vec3 movement{};

		glm::vec2 mouseMov = Input::GetMouseMovement();
		if (Input::GetMouseButton(MouseCode::ButtonRight))
		{
			// Movement
			if (Input::GetKey(KeyCode::A))
				movement.x -= 1;
			if (Input::GetKey(KeyCode::D))
				movement.x += 1;

			if (Input::GetKey(KeyCode::W))
				movement.y += 1;
			if (Input::GetKey(KeyCode::S))
				movement.y -= 1;

			if (Input::GetKey(KeyCode::Q))
				movement.z -= 1;
			if (Input::GetKey(KeyCode::E))
				movement.z += 1;

			// Camera look
			m_Yaw -= mouseMov.x * 0.3f;
			m_Pitch += mouseMov.y * 0.3f;

			Input::SetCursorMode(false);
		}
		else
		{
			Input::SetCursorMode(true);
		}

		// constrain pitch
		if (m_Pitch > 89.9f)
			m_Pitch = 89.9f;
		if (m_Pitch < -89.9f)
			m_Pitch = -89.9f;

		glm::vec3 front;
		front.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
		front.y = sin(glm::radians(m_Pitch));
		front.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(-m_Pitch));
		front = glm::normalize(front);
		m_Forward = front;

		if (glm::length(movement) > 0)
		{
			const glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
			const glm::vec3 forward = glm::normalize(m_Forward);
			const glm::vec3 right = glm::normalize(glm::cross(m_Forward, up));

			movement = glm::normalize(movement);
			glm::vec3 mov{};

			mov += movement.x * right;
			mov += movement.y * forward;
			mov += movement.z * up;

			m_Position += mov * Time::GetDeltaTime() * (m_IsSlowMovement ? m_MoveSpeedSlow : m_MoveSpeed);
		}
	}

	glm::mat4 Camera::GetView()
	{
		UpdateViewMatrix();

		return m_View;
	}

	glm::mat4 Camera::GetProjection() const
	{
		return m_Projection;
	}

	bool Camera::OnResize(WindowResizeEvent& e)
	{
		UpdateProjection(m_Fov, static_cast<float>(e.GetWidth()), static_cast<float>(e.GetHeight()), m_ZNear, m_ZFar);
		return false;
	}

	bool Camera::OnKeyPressed(KeyPressedEvent& e)
	{
		if (e.GetKeyCode() == KeyCode::LeftShift)
			m_IsSlowMovement = !m_IsSlowMovement;

		return false;
	}

	void Camera::UpdateViewMatrix()
	{
		m_View = glm::lookAt(m_Position, m_Position + m_Forward, glm::vec3(0.0f, 1.0f, 0.0f));
	}

	void Camera::UpdateProjection(float fov, float width, float height, float zNear, float zFar)
	{
		m_Fov = fov;
		m_Width = width;
		m_Height = height;
		m_ZNear = zNear;
		m_ZFar = zFar;

		m_Projection = glm::perspective(glm::radians(m_Fov / 2), m_Width / m_Height, m_ZNear, m_ZFar);
	}
}
