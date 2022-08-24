#pragma once
#include "Pelican/Events/Event.h"

namespace Pelican
{
	class Context;

	class Subsystem
	{
	public:
		Subsystem(Context* pContext) : m_pContext(pContext) {}
		virtual ~Subsystem() = default;

		virtual bool OnInitialize() { return true; }
		virtual void OnTick() {}
		virtual void OnShutdown() {}
		virtual void OnEvent(Event& /*e*/) {}

	protected:
		Context* m_pContext;
	};

	template<class T>
	concept EngineSystem = requires
	{
		std::derived_from<T, Subsystem>;
	};
}
