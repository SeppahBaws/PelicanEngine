#include "PelicanPCH.h"
#include "Application.h"
#include "Time.h"
#include "Pelican/Renderer/Camera.h"
#include "Pelican/Renderer/Mesh.h"
#include "Pelican/Renderer/Gltf/GltfModel.h"
#include "Pelican/Renderer/ImGui/ImGuiWrapper.h"

#include "Input/Input.h"

#include <thread>
#include <logtools.h>
#include <filesystem>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

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

		InitImGui();

		auto t = std::chrono::high_resolution_clock::now();
		auto lastTime = std::chrono::high_resolution_clock::now();
		while (!m_pWindow->ShouldClose())
		{
			const auto currentTime = std::chrono::high_resolution_clock::now();
			Time::Update(lastTime);
			lastTime = currentTime;

			m_pWindow->Update();

			// ImGui Update and drawing. All ImGui drawing should go in between NewFrame() and Render()
			{
				m_ImGui->UpdateIO();
				m_ImGui->NewFrame(); // ImGui elements get drawn inside of here.

				ImGui::ShowDemoWindow();

				if (ImGui::Begin("Hello Pelican Engine!"))
				{
					ImGui::Text("Hello there!");

					if (ImGui::Button("Log something"))
					{
						Logger::LogDebug("[IMGUI] Test Test!");
					}
				}
				ImGui::End();

				m_ImGui->Render();
				m_ImGui->UpdateBuffers();
			}

			m_pCamera->Update();
			
			m_pModel->Update(m_pCamera);

			m_pRenderer->BeginScene();

			// Draw scene
			{
				// m_pMesh->Draw();
				m_pModel->Draw();
			}

			// Draw UI
			{
				m_ImGui->DrawFrame();
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

		m_pWindow = new Window(Window::Params{ 1280, 720, "Sandbox", false }); // TODO: fix window resize crash.
		m_pRenderer = new VulkanRenderer();
		m_pCamera = new Camera(/*90.0f*/ 120.0f,
			static_cast<float>(m_pWindow->GetParams().width),
			static_cast<float>(m_pWindow->GetParams().height),
			0.1f, 1000.0f);

		m_pWindow->Init();
		Input::Init(m_pWindow->GetGLFWWindow());
		m_pRenderer->Initialize();

		m_pRenderer->SetCamera(m_pCamera);

		/* ===== Inside Cube ===== */
		// const std::vector<Vertex> vertices = {
		// 	// Left face
		// 	{{-5.0f, -5.0f, -5.0f}, {1.0f, 1.0f, 1.0f}},
		// 	{{ 5.0f, -5.0f, -5.0f}, {1.0f, 1.0f, 1.0f}},
		// 	{{ 5.0f,  5.0f, -5.0f}, {1.0f, 1.0f, 1.0f}},
		// 	{{-5.0f,  5.0f, -5.0f}, {1.0f, 1.0f, 1.0f}},
		//
		// 	// Right face
		// 	{{-5.0f, -5.0f,  5.0f}, {1.0f, 1.0f, 0.0f}},
		// 	{{-5.0f,  5.0f,  5.0f}, {1.0f, 1.0f, 0.0f}},
		// 	{{ 5.0f,  5.0f,  5.0f}, {1.0f, 1.0f, 0.0f}},
		// 	{{ 5.0f, -5.0f,  5.0f}, {1.0f, 1.0f, 0.0f}},
		//
		// 	// Top face
		// 	{{-5.0f,  5.0f, -5.0f}, {0.0f, 1.0f, 0.0f}},
		// 	{{ 5.0f,  5.0f, -5.0f}, {0.0f, 1.0f, 0.0f}},
		// 	{{ 5.0f,  5.0f,  5.0f}, {0.0f, 1.0f, 0.0f}},
		// 	{{-5.0f,  5.0f,  5.0f}, {0.0f, 1.0f, 0.0f}},
		//
		// 	// Bottom face
		// 	{{-5.0f, -5.0f, -5.0f}, {0.0f, 0.0f, 1.0f}},
		// 	{{-5.0f, -5.0f,  5.0f}, {0.0f, 0.0f, 1.0f}},
		// 	{{ 5.0f, -5.0f,  5.0f}, {0.0f, 0.0f, 1.0f}},
		// 	{{ 5.0f, -5.0f, -5.0f}, {0.0f, 0.0f, 1.0f}},
		//
		// 	// Front face
		// 	{{-5.0f, -5.0f, -5.0f}, {1.0f, 0.0f, 0.0f}},
		// 	{{-5.0f,  5.0f, -5.0f}, {1.0f, 0.0f, 0.0f}},
		// 	{{-5.0f,  5.0f,  5.0f}, {1.0f, 0.0f, 0.0f}},
		// 	{{-5.0f, -5.0f,  5.0f}, {1.0f, 0.0f, 0.0f}},
		//
		// 	// Back face
		// 	{{ 5.0f, -5.0f, -5.0f}, {1.0f, 0.5f, 0.0f}},
		// 	{{ 5.0f, -5.0f,  5.0f}, {1.0f, 0.5f, 0.0f}},
		// 	{{ 5.0f,  5.0f,  5.0f}, {1.0f, 0.5f, 0.0f}},
		// 	{{ 5.0f,  5.0f, -5.0f}, {1.0f, 0.5f, 0.0f}},
		// };
		// const std::vector<uint16_t> indices = {
		// 	0, 1, 2, 2, 3, 0,       // Left face
		// 	4, 5, 6, 6, 7, 4,       // Right face
		// 	8, 9, 10, 10, 11, 8,    // Top face
		// 	12, 13, 14, 14, 15, 12, // Bottom face
		// 	16, 17, 18, 18, 19, 16, // Front face
		// 	20, 21, 22, 22, 23, 20, // Back face
		// };

		/* ===== Two Planes ===== */
		// const std::vector<Vertex> vertices = {
		// 	{{-2.0f,  2.0f, -2.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
		// 	{{-2.0f,  2.0f,  2.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
		// 	{{ 2.0f,  2.0f,  2.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
		// 	{{ 2.0f,  2.0f, -2.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
		//
		// 	{{-2.0f, -2.0f, -2.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
		// 	{{-2.0f, -2.0f,  2.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
		// 	{{ 2.0f, -2.0f,  2.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
		// 	{{ 2.0f, -2.0f, -2.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
		// };
		// const std::vector<uint32_t> indices = {
		// 	0, 1, 2, 2, 3, 0,
		// 	4, 5, 6, 6, 7, 4
		// };
		
		// m_pMesh = new Mesh(vertices, indices);

		// m_pMesh = new Mesh("res/models/triangle.gltf");
		// m_pMesh = new Mesh("res/models/quad.gltf");
		// m_pMesh = new Mesh("res/models/cube.gltf");
		// m_pMesh = new Mesh("res/models/icoSphere.gltf");
		// m_pMesh = new Mesh("res/models/pony_cartoon/scene.gltf");

		// m_pMesh->CreateBuffers();

		m_pModel = new GltfModel("res/models/pony_cartoon/scene.gltf");
		// m_pModel = new GltfModel("res/models/tactical_flashlight/scene.gltf");
	}

	void Application::InitImGui()
	{
		m_ImGui = new ImGuiWrapper(m_pRenderer->GetVulkanDevice());
		const Window::Params windowParams = m_pWindow->GetParams();
		m_ImGui->Init(static_cast<float>(windowParams.width), static_cast<float>(windowParams.height));

		ImGuiInitInfo initInfo = m_pRenderer->GetImGuiInitInfo();
		m_ImGui->InitResources(initInfo.renderPass, initInfo.queue, "res/shaders/imgui");
	}

	void Application::Cleanup()
	{
		m_pRenderer->BeforeSceneCleanup();
		delete m_ImGui;

		delete m_pModel;
		// m_pMesh->Cleanup();

		m_pRenderer->AfterSceneCleanup();
		m_pWindow->Cleanup();

		// delete m_pMesh;
		delete m_pCamera;
		delete m_pRenderer;
		delete m_pWindow;

		m_pModel = nullptr;
		// m_pMesh = nullptr;
		m_pCamera = nullptr;
		m_pRenderer = nullptr;
		m_pWindow = nullptr;
	}
}
