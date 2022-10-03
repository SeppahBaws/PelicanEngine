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

	struct ApplicationSpecification
	{
		std::string name = "Pelican Application";
		u32 windowWidth = 1600;
		u32 windowHeight = 900;
		bool fullscreen = false;
		bool vsync = true;
		bool resizable = true;
		bool startMaximized = true;
	};

	class Application
	{
	public:
		Application(const ApplicationSpecification& specification);
		virtual ~Application();

		void OnEvent(Event& e);

		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);

		virtual void Run() final;

		virtual void LoadScene(Scene* /*pScene*/) {}

	public:
		static Application& Get() { return *m_Instance; }
		Scene* GetScene() const;

	public:
		RenderMode m_RenderMode = RenderMode::Filled;

	private:
		std::shared_ptr<Context> m_pContext;

		ApplicationSpecification m_Specification;

		static Application* m_Instance;
	};

	Application* CreateApplication();
}
