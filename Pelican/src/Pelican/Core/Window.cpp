#include "PelicanPCH.h"
#include "Window.h"
#include "Application.h"

#include "Pelican/Events/ApplicationEvent.h"
#include "Pelican/Events/KeyEvent.h"
#include "Pelican/Events/MouseEvent.h"

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

		// Set GLFW callbacks
		glfwSetFramebufferSizeCallback(m_pGLFWwindow, [](GLFWwindow* window, int width, int height)
		{
			Window* pWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));
			pWindow->m_Params.width = width;
			pWindow->m_Params.height = height;

			WindowResizeEvent event(width, height);
			pWindow->m_EventCallback(event);
		});

		glfwSetWindowCloseCallback(m_pGLFWwindow, [](GLFWwindow* window)
		{
			Window* pWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));

			WindowCloseEvent event;
			pWindow->m_EventCallback(event);
		});

		glfwSetWindowFocusCallback(m_pGLFWwindow, [](GLFWwindow* window, int focused)
		{
			Window* pWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));

			if (focused)
			{
				WindowGainedFocusEvent event;
				pWindow->m_EventCallback(event);
			}
			else
			{
				WindowLostFocusEvent event;
				pWindow->m_EventCallback(event);
			}
		});

		glfwSetKeyCallback(m_pGLFWwindow, [](GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/)
		{
			Window* pWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));

			switch (action)
			{
				case GLFW_PRESS:
				{
					KeyPressedEvent event(key, 0);
					pWindow->m_EventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					KeyReleasedEvent event(key);
					pWindow->m_EventCallback(event);
					break;
				}
				case GLFW_REPEAT:
				{
					KeyPressedEvent event(key, 1);
					pWindow->m_EventCallback(event);
					break;
				}
			}
		});

		glfwSetCharCallback(m_pGLFWwindow, [](GLFWwindow* window, unsigned int keycode)
		{
			Window* pWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));

			KeyTypedEvent event(keycode);
			pWindow->m_EventCallback(event);
		});

		glfwSetMouseButtonCallback(m_pGLFWwindow, [](GLFWwindow* window, int button, int action, int /*mods*/)
		{
			Window* pWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));

			switch (action)
			{
			case GLFW_PRESS:
			{
				MouseButtonPressedEvent event(button);
				pWindow->m_EventCallback(event);
				break;
			}
			case GLFW_RELEASE:
			{
				MouseButtonReleasedEvent event(button);
				pWindow->m_EventCallback(event);
				break;
			}
			}
		});

		glfwSetScrollCallback(m_pGLFWwindow, [](GLFWwindow* window, double xOffset, double yOffset)
		{
			Window* pWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));

			MouseScrolledEvent event(static_cast<float>(xOffset), static_cast<float>(yOffset));
			pWindow->m_EventCallback(event);
		});

		glfwSetCursorPosCallback(m_pGLFWwindow, [](GLFWwindow* window, double xPos, double yPos)
		{
			Window* pWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));

			MouseMovedEvent event(static_cast<float>(xPos), static_cast<float>(yPos));
			pWindow->m_EventCallback(event);
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
		// glfwSwapBuffers(m_pGLFWwindow);
	}

	bool Window::ShouldClose() const
	{
		return glfwWindowShouldClose(m_pGLFWwindow);
	}
}
