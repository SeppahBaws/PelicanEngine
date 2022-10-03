#pragma once
#include "Subsystem.h"
#include "Pelican/Events/Event.h"

struct GLFWwindow;
struct GLFWmonitor;

namespace Pelican
{
	struct WindowSpecification
	{
		std::string title = "Pelican Application";
		u32 width = 1600;
		u32 height = 900;
		bool fullscreen = false;
	};

	class Window final : public Subsystem
	{
	public:
		using EventCallbackFn = std::function<void(Event&)>;

		Window(Context* pContext, const WindowSpecification& specification);
		~Window() override;

		void SetEventCallback(const EventCallbackFn& callback) { m_EventCallback = callback; }

		bool OnInitialize() override;
		void OnTick() override;
		void OnShutdown() override;

		[[nodiscard]] bool ShouldClose() const;

		void Maximize();
		void Restore();
		void SetResizable(bool resizable);
		void SetTitle(const std::string& title);

		void Center();

		[[nodiscard]] GLFWwindow* GetHandle() const { return m_pGLFWwindow; }

		[[nodiscard]] const WindowSpecification& GetSpecification() const { return m_Specification; }

	private:
		void CenterWindow();
		GLFWmonitor* GetBestMonitor() const;

	private:
		GLFWwindow* m_pGLFWwindow;
		WindowSpecification m_Specification;

		EventCallbackFn m_EventCallback;
	};
}
