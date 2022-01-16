#pragma once
#include "Subsystem.h"
#include "Pelican/Events/Event.h"

struct GLFWwindow;
struct GLFWmonitor;

namespace Pelican
{
	class Window final : public Subsystem
	{
	public:
		using EventCallbackFn = std::function<void(Event&)>;

		struct Params
		{
			int width;
			int height;
			std::string title;
			bool resizable;
		};

		Window(Context* pContext, Params&& params);
		~Window() override;

		void SetEventCallback(const EventCallbackFn& callback) { m_EventCallback = callback; }

		bool OnInitialize() override;
		void OnTick() override;
		void OnShutdown() override;

		[[nodiscard]] bool ShouldClose() const;

		[[nodiscard]] GLFWwindow* GetGLFWWindow() const { return m_pGLFWwindow; }

		[[nodiscard]] Params GetParams() const { return m_Params; }

	private:
		void CenterWindow();
		GLFWmonitor* GetBestMonitor() const;

	private:
		GLFWwindow* m_pGLFWwindow;
		Params m_Params;

		EventCallbackFn m_EventCallback;
	};
}
