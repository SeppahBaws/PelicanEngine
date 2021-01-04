#pragma once

#include <glm/vec2.hpp>

#include "KeyCodes.h"
#include "MouseCodes.h"

struct GLFWwindow;

namespace Pelican
{
	class Input
	{
	public:
		static void Init(GLFWwindow* window);
		static void Update();

		static bool GetKey(KeyCode key);

		static bool GetMouseButton(MouseCode code);
		static glm::vec2 GetMousePos();
		static glm::vec2 GetMouseMovement();
		static float GetScroll();

		static void SetCursorMode(bool enabled);

	private:
		// Singleton
		static Input& GetInstance()
		{
			static Input instance{};
			return instance;
		}

	private:
		GLFWwindow* m_pWindow{};

		glm::vec2 m_LastMousePos{};
		float m_Scroll{};
	};
}
