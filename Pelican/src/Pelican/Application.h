﻿#pragma once
#include "Window.h"

#include "Pelican/Renderer/VulkanRenderer.h"

namespace Pelican
{
	class Mesh;
	class GltfModel;
	class ImGuiWrapper;

	class Application
	{
	public:
		Application();
		virtual ~Application() = default;

		virtual void Run() final;

		Window* GetWindow() const { return m_pWindow; }
		VulkanRenderer& GetRenderer() const { return *m_pRenderer; }
		static Application& Get() { return *m_Instance; }

	private:
		void Init();
		void InitImGui();
		void Cleanup();

	private:
		Window* m_pWindow{};
		VulkanRenderer* m_pRenderer{};
		Camera* m_pCamera{};
		// Mesh* m_pMesh{};
		GltfModel* m_pModel{};

		ImGuiWrapper* m_ImGui{};

		static Application* m_Instance;
	};

	Application* CreateApplication();
}
