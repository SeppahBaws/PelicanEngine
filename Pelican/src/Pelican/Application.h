#pragma once
#include "Window.h"

#include "VulkanRenderer.h"

namespace Pelican
{
	class Application
	{
	public:
		Application();
		virtual ~Application() = default;

		virtual void Run() final;

		Window* GetWindow() const { return m_pWindow; }
		static Application& Get() { return *m_Instance; }

	private:
		Window* m_pWindow{};
		VulkanRenderer m_Renderer;

		static Application* m_Instance;
	};

	Application* CreateApplication();
}
