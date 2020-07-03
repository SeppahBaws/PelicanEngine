#include "PelicanPCH.h"
#include "Application.h"

namespace Pelican
{
	Application* Application::m_Instance = nullptr;

	Application::Application()
	{
		if (m_Instance)
		{
			ASSERT_MSG(false, "Application already exists! Cannot create two applications at once!");
		}

		m_Instance = this;
	}

	void Application::Run()
	{
		m_pWindow = new Window(Window::Params{ 1280, 720, "Hello Pelican!", false });

		m_pWindow->Init();
		m_Renderer.Initialize();

		while (!m_pWindow->ShouldClose())
		{
			m_Renderer.Draw();
			m_pWindow->Update();
		}

		m_Renderer.Cleanup();
		m_pWindow->Cleanup();
		delete m_pWindow;
	}
}
