#include "PelicanPCH.h"
#include "Application.h"

#include "Pelican/Core/Time.h"
#include "Pelican/Input/Input.h"
#include "Pelican/Renderer/Camera.h"
#include "Pelican/Renderer/Mesh.h"
#include "Pelican/Renderer/Gltf/GltfModel.h"
#include "Pelican/Renderer/ImGui/ImGuiWrapper.h"
#include "Pelican/Scene/Scene.h"

#include <thread>
#include <logtools.h>
#include <filesystem>

#include <imgui.h>

#include "Layer.h"
#include "Pelican/Events/ApplicationEvent.h"
#include "Pelican/Events/MouseEvent.h"

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

	void Application::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(Application::OnWindowClose));
		dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(Application::OnWindowResize));

		if (e.IsInCategory(EventCategoryApplication))
		{
			const std::string s = e.ToString();
			Logger::LogDebug(s);
		}

		for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it)
		{
			if (e.Handled)
				break;
			(*it)->OnEvent(e);
		}
	}

	void Application::PushLayer(Layer* layer)
	{
		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer* overlay)
	{
		m_LayerStack.PushOverlay(overlay);
		overlay->OnAttach();
	}

	bool Application::OnWindowClose(WindowCloseEvent& /*e*/)
	{
		return true;
	}

	bool Application::OnWindowResize(WindowResizeEvent& /*e*/)
	{
		m_pRenderer->FlagWindowResized();

		return false;
	}

	void Application::Run()
	{
		Init();

		LoadScene(m_pScene);

		auto t = std::chrono::high_resolution_clock::now();
		auto lastTime = std::chrono::high_resolution_clock::now();
		while (!m_pWindow->ShouldClose())
		{
			const auto currentTime = std::chrono::high_resolution_clock::now();
			Time::Update(lastTime);
			lastTime = currentTime;

			m_pWindow->Update();

			// Update scene
			{
				// m_pCamera->Update();
				m_pScene->Update();
				// m_pModel->Update(m_pCamera);
			}

			for (Layer* layer : m_LayerStack)
				layer->OnUpdate();

			if (!m_pRenderer->BeginScene())
				continue;

			// Draw scene
			{
				// m_pModel->Draw();
				m_pScene->Draw();
			}

			// Draw ImGui
			{
				ImGui::ShowDemoWindow();

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

				for (Layer* layer : m_LayerStack)
					layer->OnImGuiRender();
			}

			m_pRenderer->EndScene();

			Input::Update();
		}

		Cleanup();
	}

	void Application::Init()
	{
		Logger::Init();
		Logger::Configure({ true, true });

		m_pWindow = new Window(Window::Params{ 1280, 720, "Sandbox", true });
		m_pRenderer = new VulkanRenderer();
		// m_pCamera = new Camera(/*90.0f*/ 120.0f,
		// 	static_cast<float>(m_pWindow->GetParams().width),
		// 	static_cast<float>(m_pWindow->GetParams().height),
		// 	0.1f, 1000.0f);

		m_pWindow->Init();
		m_pWindow->SetEventCallback(BIND_EVENT_FN(Application::OnEvent));
		Input::Init(m_pWindow->GetGLFWWindow());
		m_pRenderer->Initialize();

		// m_pRenderer->SetCamera(m_pCamera);

		m_pScene = new Scene();
		// m_pModel = new GltfModel("res/models/pony_cartoon/scene.gltf");
	}

	void Application::Cleanup()
	{
		m_pRenderer->BeforeSceneCleanup();

		delete m_pScene;
		// delete m_pModel;

		m_pRenderer->AfterSceneCleanup();
		m_pWindow->Cleanup();

		// delete m_pCamera;
		delete m_pRenderer;
		delete m_pWindow;

		m_pScene = nullptr;
		// m_pModel = nullptr;
		// m_pCamera = nullptr;
		m_pRenderer = nullptr;
		m_pWindow = nullptr;
	}
}
