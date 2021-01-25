#pragma once
#include "Window.h"

#include "Pelican/Renderer/VulkanRenderer.h"

namespace Pelican
{
	class Scene;
	class Mesh;
	class GltfModel;
	class ImGuiWrapper;

	class Application
	{
	public:
		Application();
		virtual ~Application() = default;

		virtual void Run() final;

		virtual void LoadScene(Scene* pScene) = 0;

	public:
		Window* GetWindow() const { return m_pWindow; }
		VulkanRenderer& GetRenderer() const { return *m_pRenderer; }
		static Application& Get() { return *m_Instance; }

	private:
		void Init();
		void Cleanup();

	private:
		Window* m_pWindow{};
		VulkanRenderer* m_pRenderer{};
		Scene* m_pScene{};

		static Application* m_Instance;
	};

	Application* CreateApplication();
}
