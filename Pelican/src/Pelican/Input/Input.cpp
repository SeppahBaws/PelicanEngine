#include "PelicanPCH.h"
#include "Input.h"

#include <GLFW/glfw3.h>

#include "Pelican/Core/Context.h"
#include "Pelican/Core/Window.h"

namespace Pelican
{
	Input::Input(Context* pContext)
		: Subsystem(pContext)
	{
	}

	bool Input::OnInitialize()
	{
		m_pWindow = m_pContext->GetSubsystem<Window>()->GetHandle();
		static Input* input = this;

		glfwSetScrollCallback(m_pWindow, [](GLFWwindow* /*pWindow*/, f64 x, f64 y)
		{
			input->m_Scroll = { x, y };
		});

		return true;
	}

	void Input::OnTick()
	{
		m_Scroll = glm::vec2{ 0.0f };
	}

	bool Input::GetKey(KeyCode key) const
	{
		return glfwGetKey(m_pWindow, static_cast<int>(key)) == GLFW_PRESS;
	}

	bool Input::GetMouseButton(MouseCode code) const
	{
		return glfwGetMouseButton(m_pWindow, static_cast<int>(code)) == GLFW_PRESS;
	}

	glm::vec2 Input::GetMousePos() const
	{
		f64 x, y;
		glfwGetCursorPos(m_pWindow, &x, &y);

		return glm::vec2(static_cast<float>(x), static_cast<float>(y));
	}

	glm::vec2 Input::GetMouseMovement()
	{
		f64 x, y;
		glfwGetCursorPos(m_pWindow, &x, &y);

		const glm::vec2 mov = m_LastMousePos - glm::vec2(x, y);

		m_LastMousePos = { x, y };

		return mov;
	}

	glm::vec2 Input::GetScroll() const
	{
		return m_Scroll;
	}

	void Input::SetCursorMode(bool enabled)
	{
		glfwSetInputMode(m_pWindow, GLFW_CURSOR, enabled ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
	}
}
