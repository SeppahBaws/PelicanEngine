#include "PelicanPCH.h"
#include "Application.h"
#include <iostream>

namespace Pelican
{
	void Application::Run()
	{
		m_pWindow = new Window(Window::Params{ 1280, 720, "Hello Pelican!", false });

		try
		{
			m_pWindow->Init();
			m_Device.Initialize();
		}
		catch (std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}

		while (!m_pWindow->ShouldClose())
		{
			m_pWindow->Update();
		}

		m_Device.Cleanup();
		m_pWindow->Cleanup();
		delete m_pWindow;
	}
}
