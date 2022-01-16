#include "PelicanPCH.h"
#include "Application.h"

#include "Pelican/Core/Time.h"
#include "Pelican/Input/Input.h"
#include "Pelican/Renderer/Camera.h"
#include "Pelican/Renderer/Mesh.h"
#include "Pelican/Renderer/Model.h"
#include "Pelican/Renderer/ImGui/ImGuiWrapper.h"
#include "Pelican/Scene/Scene.h"

#include <thread>
#include <logtools.h>

#include <imgui.h>

#include "Pelican/Events/ApplicationEvent.h"

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


		Logger::Init();
		Logger::Configure({ true, true });


		m_pContext = std::make_shared<Context>();

		m_pContext->AddSubsystem<Window>(Window::Params{ 1600, 900, "Sandbox", true });
		m_pContext->AddSubsystem<VulkanRenderer>();

		if (!m_pContext->OnInitialize())
		{
			Logger::LogInfo("Failed to initialize some subsystems!");
		}
		else
		{
			Logger::LogInfo("Initialized all subsystems!");
		}


		// TODO: camera should be fetched from the scene.
		Window* pWindow = m_pContext->GetSubsystem<Window>();
		VulkanRenderer* pRenderer = m_pContext->GetSubsystem<VulkanRenderer>();
		m_pCamera = new Camera(120.0f,
			static_cast<float>(pWindow->GetParams().width),
			static_cast<float>(pWindow->GetParams().height),
			0.1f, 1000.0f);
		pRenderer->SetCamera(m_pCamera);

		pWindow->SetEventCallback(BIND_EVENT_FN(Application::OnEvent));
		Input::Init(pWindow->GetGLFWWindow());

		m_pScene = new Scene();
	}

	Application::~Application()
	{
		m_pContext->GetSubsystem<VulkanRenderer>()->WaitForIdle();

		delete m_pScene;
		delete m_pCamera;

		m_pScene = nullptr;
		m_pCamera = nullptr;

		m_pContext->OnShutdown();
	}

	void Application::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(Application::OnWindowClose));
		dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(Application::OnWindowResize));

		m_pCamera->OnEvent(e);

		if (e.IsInCategory(EventCategoryApplication))
		{
			const std::string s = e.ToString();
			Logger::LogDebug(s);
		}
	}

	bool Application::OnWindowClose(WindowCloseEvent& /*e*/)
	{
		return true;
	}

	bool Application::OnWindowResize(WindowResizeEvent& /*e*/)
	{
		m_pContext->GetSubsystem<VulkanRenderer>()->FlagWindowResized();

		return false;
	}

	void Application::Run()
	{
		LoadScene(m_pScene);

		Window* pWindow = m_pContext->GetSubsystem<Window>();
		VulkanRenderer* pRenderer = m_pContext->GetSubsystem<VulkanRenderer>();

		auto t = std::chrono::high_resolution_clock::now();
		auto lastTime = std::chrono::high_resolution_clock::now();
		while (!pWindow->ShouldClose())
		{
			const auto currentTime = std::chrono::high_resolution_clock::now();
			Time::Update(lastTime);
			lastTime = currentTime;

			m_pContext->OnTick();

			m_pCamera->Update();

			// Update scene
			{
				m_pScene->Update(m_pCamera);
			}

			if (!pRenderer->BeginScene())
				continue;

			// Draw scene
			{
				m_pScene->Draw(m_pCamera);
			}

			// Draw ImGui
			{
				// Show some frame time results.
				ImGui::SetNextWindowPos(ImVec2(10.0f, 10.0f));
				ImGui::SetNextWindowBgAlpha(0.35f);
				if (ImGui::Begin("Frame timings", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove))
				{
					ImGui::Text("Frame time: %fms", Time::GetDeltaTime() * 1000.0f);
					ImGui::Text("Fps: %.0f", 1.0f / Time::GetDeltaTime());
					glm::vec2 pos = Input::GetMousePos();
					ImGui::Text("Mouse position: (%.0f, %.0f)", pos.x, pos.y);
				}
				ImGui::End();

				if (ImGui::Begin("Shader Reloader"))
				{
					if (ImGui::Button("Reload Shaders!"))
					{
						pRenderer->ReloadShaders();
					}
				}
				ImGui::End();

				if (ImGui::Begin("Renderer Settings"))
				{
					if (ImGui::CollapsingHeader("Rasterization Settings"))
					{
						ImGui::Text("Rendering Mode");

						int mode = static_cast<int>(m_RenderMode);
						ImGui::RadioButton("Solid", &mode, static_cast<int>(RenderMode::Filled));
						ImGui::RadioButton("Lines", &mode, static_cast<int>(RenderMode::Lines));
						ImGui::RadioButton("Points", &mode, static_cast<int>(RenderMode::Points));
						m_RenderMode = static_cast<RenderMode>(mode);
					}
				}
				ImGui::End();
			}

			pRenderer->EndScene();

			Input::Update();
		}
	}
}
