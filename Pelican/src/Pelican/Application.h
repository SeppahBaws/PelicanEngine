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
		VulkanRenderer& GetRenderer() const { return *m_pRenderer; }
		static Application& Get() { return *m_Instance; }

	private:
		void Init();
		void Cleanup();

	private:
		Window* m_pWindow{};
		VulkanRenderer* m_pRenderer{};
		Camera* m_pCamera{};

		static Application* m_Instance;
	};

	Application* CreateApplication();
}
