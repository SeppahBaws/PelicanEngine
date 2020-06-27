#pragma once
#include "Window.h"

#include "VulkanDevice.h"

namespace Pelican
{
	class Application
	{
	public:
		Application() = default;
		virtual ~Application() = default;

		virtual void Run() final;

	private:
		Window* m_pWindow;
		VulkanDevice m_Device;
	};

	Application* CreateApplication();
}
