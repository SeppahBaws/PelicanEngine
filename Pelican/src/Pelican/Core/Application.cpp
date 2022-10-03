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
	Application* Application::m_Instance{};

	Application::Application(const ApplicationSpecification& specification)
	{
		if (m_Instance)
		{
			ASSERT_MSG(false, "Application already exists! Cannot create two applications at once!");
		}
		m_Instance = this;

		m_Specification = specification;

		Logger::Init();
		Logger::Configure({ true, true });


		m_pContext = std::make_shared<Context>();

		WindowSpecification windowSpec = {
			.title = m_Specification.name,
			.width = m_Specification.windowWidth,
			.height = m_Specification.windowHeight,
			.fullscreen = m_Specification.fullscreen
		};
		Window* pWindow = m_pContext->AddSubsystem<Window>(windowSpec);
		m_pContext->AddSubsystem<Input>();
		m_pContext->AddSubsystem<VulkanRenderer>();
		m_pContext->AddSubsystem<Scene>();

		if (!m_pContext->OnInitialize())
		{
			Logger::LogInfo("Failed to initialize some subsystems!");
		}
		else
		{
			Logger::LogInfo("Initialized all subsystems!");
		}

		// if (m_Specification.startMaximized)
		// {
		// 	pWindow->Maximize();
		// }
		// pWindow->SetResizable(m_Specification.resizable);
		
		pWindow->SetEventCallback(BIND_EVENT_FN(Application::OnEvent));

	}

	Application::~Application()
	{
		m_pContext->GetSubsystem<VulkanRenderer>()->WaitForIdle();

		m_pContext->OnShutdown();
	}

	void Application::OnEvent(Event& e)
	{
		m_pContext->OnEvent(e);

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(Application::OnWindowClose));
		dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(Application::OnWindowResize));

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
		Scene* pScene = m_pContext->GetSubsystem<Scene>();
		LoadScene(pScene);

		Window* pWindow = m_pContext->GetSubsystem<Window>();
		Input* pInput = m_pContext->GetSubsystem<Input>();
		VulkanRenderer* pRenderer = m_pContext->GetSubsystem<VulkanRenderer>();

		auto t = std::chrono::high_resolution_clock::now();
		auto lastTime = std::chrono::high_resolution_clock::now();
		while (!pWindow->ShouldClose())
		{
			const auto currentTime = std::chrono::high_resolution_clock::now();
			Time::Update(lastTime);
			lastTime = currentTime;

			m_pContext->OnTick();

			// Draw ImGui
			{
				// Show some frame time results.
				ImGui::SetNextWindowPos(ImVec2(10.0f, 10.0f));
				ImGui::SetNextWindowBgAlpha(0.35f);
				if (ImGui::Begin("Frame timings", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove))
				{
					ImGui::Text("Frame time: %fms", Time::GetDeltaTime() * 1000.0f);
					ImGui::Text("Fps: %.0f", 1.0f / Time::GetDeltaTime());
					const glm::vec2 pos = pInput->GetMousePos();
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
		}
	}

	Scene* Application::GetScene() const
	{
		Scene* pScene = m_pContext->GetSubsystem<Scene>();
		return pScene;
	}
}
