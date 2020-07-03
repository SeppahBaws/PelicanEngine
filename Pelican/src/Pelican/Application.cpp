#include "PelicanPCH.h"
#include "Application.h"
#include <iostream>

namespace Pelican
{
	Application* Application::m_Instance = nullptr;

	Application::Application()
	{
		if (m_Instance)
		{
			throw std::runtime_error("Application already exists! Cannot create two applications at once!");
		}

		m_Instance = this;
	}

	void Application::Run()
	{
		m_pWindow = new Window(Window::Params{ 1280, 720, "Hello Pelican!", false });

		try
		{
			m_pWindow->Init();
			m_Renderer.Initialize();
		}
		catch (std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}

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
