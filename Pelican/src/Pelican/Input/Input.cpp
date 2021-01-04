#include "PelicanPCH.h"
#include "Input.h"

#include <GLFW/glfw3.h>

namespace Pelican
{
	void Input::Init(GLFWwindow* window)
	{
		GetInstance().m_pWindow = window;

		glfwSetScrollCallback(window, [](GLFWwindow* /*pWindow*/, double /*x*/, double y)
		{
			GetInstance().m_Scroll = static_cast<float>(y);
		});
	}

	void Input::Update()
	{
		GetInstance().m_Scroll = 0.0f;
	}

	bool Input::GetKey(KeyCode key)
	{
		return glfwGetKey(GetInstance().m_pWindow, static_cast<int>(key)) == GLFW_PRESS;
	}

	bool Input::GetMouseButton(MouseCode code)
	{
		return glfwGetMouseButton(GetInstance().m_pWindow, static_cast<int>(code)) == GLFW_PRESS;
	}

	glm::vec2 Input::GetMousePos()
	{
		double x, y;
		glfwGetCursorPos(GetInstance().m_pWindow, &x, &y);

		return glm::vec2(static_cast<float>(x), static_cast<float>(y));
	}

	glm::vec2 Input::GetMouseMovement()
	{
		double x, y;
		glfwGetCursorPos(GetInstance().m_pWindow, &x, &y);

		const glm::vec2 mov = GetInstance().m_LastMousePos - glm::vec2(x, y);

		GetInstance().m_LastMousePos = { x, y };

		return mov;
	}

	float Input::GetScroll()
	{
		return GetInstance().m_Scroll;
	}

	void Input::SetCursorMode(bool enabled)
	{
		glfwSetInputMode(GetInstance().m_pWindow, GLFW_CURSOR, enabled ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
	}
}
