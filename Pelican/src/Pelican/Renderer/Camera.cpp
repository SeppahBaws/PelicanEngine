#include "PelicanPCH.h"
#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.inl>

#include "Pelican/Core/Time.h"
#include "Pelican/Events/ApplicationEvent.h"
#include "Pelican/Events/Event.h"
#include "Pelican/Events/KeyEvent.h"
#include "Pelican/Input/Input.h"

namespace Pelican
{
	Camera::Camera(Context* pContext, f32 fov, f32 width, f32 height, f32 zNear, f32 zFar)
		: m_pContext(pContext)
		, m_Position(-20.0f, 20.0f, 20.0f)
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
		Input* pInput = m_pContext->GetSubsystem<Input>();

		glm::vec3 moveInput{};

		glm::vec2 mouseMov = pInput->GetMouseMovement();
		if (pInput->GetMouseButton(MouseCode::ButtonRight))
		{
			// Movement
			if (pInput->GetKey(KeyCode::A))
				moveInput.x -= 1;
			if (pInput->GetKey(KeyCode::D))
				moveInput.x += 1;

			if (pInput->GetKey(KeyCode::W))
				moveInput.y += 1;
			if (pInput->GetKey(KeyCode::S))
				moveInput.y -= 1;

			if (pInput->GetKey(KeyCode::Q))
				moveInput.z -= 1;
			if (pInput->GetKey(KeyCode::E))
				moveInput.z += 1;

			// Camera look
			m_Yaw -= mouseMov.x * Time::GetDeltaTime() * m_LookSpeed;
			m_Pitch += mouseMov.y * Time::GetDeltaTime() * m_LookSpeed;

			pInput->SetCursorMode(false);
		}
		else
		{
			pInput->SetCursorMode(true);
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

		const glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
		const glm::vec3 forward = glm::normalize(m_Forward);
		const glm::vec3 right = glm::normalize(glm::cross(m_Forward, up));

		if (glm::length(moveInput) > 0)
		{
			moveInput = glm::normalize(moveInput);

			m_Velocity = {};
			m_Velocity += moveInput.x * right;
			m_Velocity += moveInput.y * forward;
			m_Velocity += moveInput.z * up;

			m_Velocity *= m_IsSlowMovement ? m_MoveSpeedSlow : m_MoveSpeed;
		}
		else
		{
			f32 friction = std::clamp(m_Friction, 0.0f, 1.0f);
			m_Velocity *= 1.0f - friction;

			if (glm::length2(m_Velocity) < F32_EPSILON)
			{
				m_Velocity = glm::vec3{ 0.0f };
			}
		}

		m_Position += m_Velocity * Time::GetDeltaTime();
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

	glm::vec3 Camera::GetPosition() const
	{
		return m_Position;
	}

	bool Camera::OnResize(WindowResizeEvent& e)
	{
		if (e.GetWidth() <= 0 || e.GetHeight() <= 0)
			return false;

		UpdateProjection(m_Fov, static_cast<f32>(e.GetWidth()), static_cast<f32>(e.GetHeight()), m_ZNear, m_ZFar);
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

	void Camera::UpdateProjection(f32 fov, f32 width, f32 height, f32 zNear, f32 zFar)
	{
		m_Fov = fov;
		m_Width = width;
		m_Height = height;
		m_ZNear = zNear;
		m_ZFar = zFar;

		m_Projection = glm::perspective(glm::radians(m_Fov / 2), m_Width / m_Height, m_ZNear, m_ZFar);
	}
}
