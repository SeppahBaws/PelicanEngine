#include "PelicanPCH.h"
#include "Window.h"
#include "Application.h"

#include "Pelican/Events/ApplicationEvent.h"
#include "Pelican/Events/KeyEvent.h"
#include "Pelican/Events/MouseEvent.h"

#include <GLFW/glfw3.h>


namespace Pelican
{
	Window::Window(Context* pContext, Params&& params)
		: Subsystem(pContext)
		, m_pGLFWwindow(nullptr)
		, m_Params(std::move(params))
	{
	}

	Window::~Window()
	{
	}

	bool Window::OnInitialize()
	{
		if (!glfwInit())
		{
			ASSERT_MSG(false, "Failed to initialize GLFW!");
			return false;
		}

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, m_Params.resizable ? GLFW_TRUE : GLFW_FALSE);

		m_pGLFWwindow = glfwCreateWindow(m_Params.width, m_Params.height, m_Params.title.c_str(), nullptr, nullptr);
		if (!m_pGLFWwindow)
		{
			glfwTerminate();
			ASSERT_MSG(false, "Failed to create GLFW window!");
			return false;
		}

		CenterWindow();

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
					KeyPressedEvent event(static_cast<KeyCode>(key), 0);
					pWindow->m_EventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					KeyReleasedEvent event(static_cast<KeyCode>(key));
					pWindow->m_EventCallback(event);
					break;
				}
				case GLFW_REPEAT:
				{
					KeyPressedEvent event(static_cast<KeyCode>(key), 1);
					pWindow->m_EventCallback(event);
					break;
				}
			}
		});

		glfwSetCharCallback(m_pGLFWwindow, [](GLFWwindow* window, unsigned int keycode)
		{
			Window* pWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));

			KeyTypedEvent event(static_cast<KeyCode>(keycode));
			pWindow->m_EventCallback(event);
		});

		glfwSetMouseButtonCallback(m_pGLFWwindow, [](GLFWwindow* window, int button, int action, int /*mods*/)
		{
			Window* pWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));

			switch (action)
			{
			case GLFW_PRESS:
			{
				MouseButtonPressedEvent event(static_cast<MouseCode>(button));
				pWindow->m_EventCallback(event);
				break;
			}
			case GLFW_RELEASE:
			{
				MouseButtonReleasedEvent event(static_cast<MouseCode>(button));
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

		return true;
	}

	void Window::OnTick()
	{
		glfwPollEvents();
	}

	void Window::OnShutdown()
	{
		glfwDestroyWindow(m_pGLFWwindow);
		glfwTerminate();
	}

	bool Window::ShouldClose() const
	{
		return glfwWindowShouldClose(m_pGLFWwindow);
	}

	// Credits to ValentinDev: https://vallentin.dev/2014/02/07/glfw-center-window
	void Window::CenterWindow()
	{
		GLFWmonitor* monitor = GetBestMonitor();
		if (!monitor)
			return;

		const GLFWvidmode* mode = glfwGetVideoMode(monitor);
		if (!mode)
			return;

		int monitorX, monitorY;
		glfwGetMonitorPos(monitor, &monitorX, &monitorY);

		int windowWidth, windowHeight;
		glfwGetWindowSize(m_pGLFWwindow, &windowWidth, &windowHeight);

		glfwSetWindowPos(m_pGLFWwindow,
			monitorX + (mode->width - windowWidth) / 2,
			monitorY + (mode->height - windowHeight) / 2);
	}

	// Credits to ValentinDev: https://vallentin.dev/2014/02/07/glfw-center-window
	GLFWmonitor* Window::GetBestMonitor() const
	{
		int monitorCount;
		GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);

		if (!monitors)
			return nullptr;

		int windowX, windowY, windowWidth, windowHeight;
		glfwGetWindowSize(m_pGLFWwindow, &windowWidth, &windowHeight);
		glfwGetWindowPos(m_pGLFWwindow, &windowX, &windowY);

		GLFWmonitor* bestMonitor = nullptr;
		int bestArea = 0;

		for (int i = 0; i < monitorCount; i++)
		{
			GLFWmonitor* monitor = monitors[i];

			int monitorX, monitorY;
			glfwGetMonitorPos(monitor, &monitorX, &monitorY);

			const GLFWvidmode* mode = glfwGetVideoMode(monitor);
			if (!mode)
				continue;

			const int areaMinX = std::max(windowX, monitorX);
			const int areaMinY = std::max(windowY, monitorY);

			const int areaMaxX = std::min(windowX + windowWidth, monitorX + mode->width);
			const int areaMaxY = std::min(windowY + windowHeight, monitorY + mode->height);

			const int area = (areaMaxX - areaMinX) * (areaMaxY - areaMinY);

			if (area > bestArea)
			{
				bestArea = area;
				bestMonitor = monitor;
			}
		}

		return bestMonitor;
	}
}
