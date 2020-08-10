#include "PelicanPCH.h"
#include "Application.h"
#include "Time.h"
#include "Camera.h"

#include "Input/Input.h"

#include <thread>
#include <logtools.h>

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
		Init();

		auto t = std::chrono::high_resolution_clock::now();
		auto lastTime = std::chrono::high_resolution_clock::now();
		while (!m_pWindow->ShouldClose())
		{
			const auto currentTime = std::chrono::high_resolution_clock::now();
			Time::Update(lastTime);

			m_pWindow->Update();

			m_pCamera->Update();

			m_pRenderer->Draw();

			t = lastTime + std::chrono::milliseconds(16);
			lastTime = currentTime;
			std::this_thread::sleep_until(t);
		}

		Cleanup();
	}

	void Application::Init()
	{
		Logger::Init();
		Logger::Configure({ true, true });

		m_pWindow = new Window(Window::Params{ 1280, 720, "Sandbox", true });
		m_pRenderer = new VulkanRenderer();
		m_pCamera = new Camera(/*90.0f*/ 120.0f,
			static_cast<float>(m_pWindow->GetParams().width),
			static_cast<float>(m_pWindow->GetParams().height),
			0.1f, 1000.0f);

		m_pWindow->Init();
		Input::Init(m_pWindow->GetGLFWWindow());
		m_pRenderer->Initialize();

		m_pRenderer->SetCamera(m_pCamera);
	}

	void Application::Cleanup()
	{
		m_pRenderer->Cleanup();
		m_pWindow->Cleanup();

		delete m_pCamera;
		delete m_pRenderer;
		delete m_pWindow;
	}
}
