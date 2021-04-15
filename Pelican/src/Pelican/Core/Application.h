#pragma once
#include "Window.h"
#include "LayerStack.h"

#include "Pelican/Events/ApplicationEvent.h"
#include "Pelican/Events/Event.h"

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

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);

		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);

		virtual void Run() final;

		virtual void LoadScene(Scene* /*pScene*/) {}

	public:
		Window* GetWindow() const { return m_pWindow; }
		VulkanRenderer& GetRenderer() const { return *m_pRenderer; }
		static Application& Get() { return *m_Instance; }
		Scene* GetScene() const { return m_pScene; }

	private:
		void Init();
		void Cleanup();

	private:
		Window* m_pWindow{};
		VulkanRenderer* m_pRenderer{};
		Scene* m_pScene{};

		LayerStack m_LayerStack;

		static Application* m_Instance;
	};

	Application* CreateApplication();
}
