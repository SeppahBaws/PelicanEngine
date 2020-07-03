#pragma once

struct GLFWwindow;

namespace Pelican
{
	class Window final
	{
	public:
		struct Params
		{
			int width;
			int height;
			std::string title;
			bool resizable;
		};

		Window(Params&& params);
		~Window();

		void Init();
		void Cleanup();

		void Update();

		[[nodiscard]] bool ShouldClose() const;

		[[nodiscard]] GLFWwindow* GetGLFWWindow() const { return m_pGLFWwindow; }

		[[nodiscard]] Params GetParams() const { return m_Params; }

	private:
		GLFWwindow* m_pGLFWwindow;
		Params m_Params;
	};
}
