#include "PelicanPCH.h"
#include "Window.h"
#include "Application.h"

#include <GLFW/glfw3.h>

namespace Pelican
{
	Window::Window(Params&& params)
		: m_pGLFWwindow(nullptr)
		, m_Params(std::move(params))
	{
	}

	Window::~Window()
	{
	}

	void Window::Init()
	{
		if (!glfwInit())
		{
			ASSERT_MSG(false, "Failed to initialize GLFW!");
		}

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, m_Params.resizable ? GLFW_TRUE : GLFW_FALSE);

		m_pGLFWwindow = glfwCreateWindow(m_Params.width, m_Params.height, m_Params.title.c_str(), nullptr, nullptr);
		if (!m_pGLFWwindow)
		{
			glfwTerminate();
			throw std::exception("Failed to create GLFW window!");
		}

		glfwSetWindowUserPointer(m_pGLFWwindow, this);

		glfwSetFramebufferSizeCallback(m_pGLFWwindow, [](GLFWwindow* window, int width, int height)
		{
			Window* pWindow = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
			pWindow->m_Params.width = width;
			pWindow->m_Params.height = height;

			Application::Get().GetRenderer().FlagWindowResized();
		});
	}

	void Window::Cleanup()
	{
		glfwDestroyWindow(m_pGLFWwindow);
		glfwTerminate();
	}

	void Window::Update()
	{
		glfwPollEvents();
		glfwSwapBuffers(m_pGLFWwindow);
	}

	bool Window::ShouldClose() const
	{
		return glfwWindowShouldClose(m_pGLFWwindow);
	}
}
