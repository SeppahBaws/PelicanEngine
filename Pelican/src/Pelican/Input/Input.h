#pragma once

#include <glm/vec2.hpp>

#include "KeyCodes.h"
#include "MouseCodes.h"
#include "Pelican/Core/Subsystem.h"

struct GLFWwindow;

namespace Pelican
{
	class Input final : public Subsystem
	{
	public:
		explicit Input(Context* pContext);
		~Input() override = default;
		bool OnInitialize() override;
		void OnTick() override;

		[[nodiscard]] bool GetKey(KeyCode key) const;
		[[nodiscard]] bool GetMouseButton(MouseCode code) const;
		[[nodiscard]] glm::vec2 GetMousePos() const;
		[[nodiscard]] glm::vec2 GetMouseMovement();
		[[nodiscard]] glm::vec2 GetScroll() const;

		void SetCursorMode(bool enabled);

	private:
		GLFWwindow* m_pWindow{};

		glm::vec2 m_LastMousePos{};
		glm::vec2 m_Scroll{};
	};
}
