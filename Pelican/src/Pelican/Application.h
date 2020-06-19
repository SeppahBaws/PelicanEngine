#pragma once

namespace Pelican
{
	class Application
	{
	public:
		Application() = default;
		virtual ~Application() = default;

		virtual void Run() final;
	};

	Application* CreateApplication();
}
