#include "PelicanPCH.h"
#include "Window.h"

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
			throw std::exception("Failed to initialize GLFW!");
		}

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, m_Params.resizable ? GLFW_TRUE : GLFW_FALSE);

		m_pGLFWwindow = glfwCreateWindow(m_Params.width, m_Params.height, m_Params.title.c_str(), nullptr, nullptr);
		if (!m_pGLFWwindow)
		{
			glfwTerminate();
			throw std::exception("Failed to create GLFW window!");
		}

		glfwMakeContextCurrent(m_pGLFWwindow);
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
