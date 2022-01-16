#pragma once
#include "Context.h"
#include "Window.h"

#include "Pelican/Events/ApplicationEvent.h"
#include "Pelican/Events/Event.h"

#include "Pelican/Renderer/VulkanRenderer.h"

namespace Pelican
{
	class Scene;
	class Mesh;
	class Model;
	class ImGuiWrapper;

	class Application
	{
	public:
		Application();
		virtual ~Application();

		void OnEvent(Event& e);

		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);

		virtual void Run() final;

		virtual void LoadScene(Scene* /*pScene*/) {}

	public:
		static Application& Get() { return *m_Instance; }
		Scene* GetScene() const { return m_pScene; }
		Camera* GetCamera() const { return m_pCamera; }

	public:
		RenderMode m_RenderMode = RenderMode::Filled;

	private:
		std::shared_ptr<Context> m_pContext;

		// TODO: make these subsystems
		Scene* m_pScene{};

		Camera* m_pCamera{};

		static Application* m_Instance;
	};

	Application* CreateApplication();
}
