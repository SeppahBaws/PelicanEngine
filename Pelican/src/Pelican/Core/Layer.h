#pragma once

#include "Pelican/Events/Event.h"

namespace Pelican
{
	class Layer
	{
	public:
		Layer(const std::string& name = "Layer");
		virtual ~Layer() = default;

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate() {}
		virtual void OnImGuiRender() {}
		virtual void OnEvent(Event& /*e*/) {}

		[[nodiscard]] const std::string& GetName() const { return m_DebugName; }

	private:
		std::string m_DebugName;
	};
}
