#include "PelicanPCH.h"
#include "Camera.h"
#include "Pelican/Core/Time.h"

#include "Pelican/Input/Input.h"

#include <iostream>

#include <glm/gtc/matrix_transform.hpp>
#include <logtools.h>
#include <imgui.h>

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

	void Camera::Update()
	{
		glm::vec3 movement{};

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

		glm::vec2 mouseMov = Input::GetMouseMovement();
		if (Input::GetMouseButton(MouseCode::ButtonRight))
		{
			// m_Yaw -= mouseMov.x * Time::GetDeltaTime() * m_MouseSpeed;
			// m_Pitch += mouseMov.y * Time::GetDeltaTime() * m_MouseSpeed;

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

			m_Position += mov * Time::GetDeltaTime() * m_MoveSpeed;
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

	void Camera::UpdateProjection(float fov, float width, float height, float zNear, float zFar)
	{
		if (fov != 0.0f)
			m_Fov = fov;
		if (width != 0.0f)
			m_Width = width;
		if (height != 0.0f)
			m_Height = height;
		if (zNear != 0.0f)
			m_ZNear = zNear;
		if (zFar != 0.0f)
			m_ZFar = zFar;

		m_Projection = glm::perspective(glm::radians(m_Fov / 2), m_Width / m_Height, m_ZNear, m_ZFar);
	}

	void Camera::UpdateViewMatrix()
	{
		m_View = glm::lookAt(m_Position, m_Position + m_Forward, glm::vec3(0.0f, 1.0f, 0.0f));
	}
}
