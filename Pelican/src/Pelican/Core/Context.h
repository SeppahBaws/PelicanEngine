#pragma once

#include <ranges>

#include "Subsystem.h"

namespace Pelican
{
	class Context final
	{
	public:
		Context() = default;

		template<EngineSystem T, class... TArgs>
		T* AddSubsystem(TArgs&&... args)
		{
			m_Subsystems.emplace_back(std::make_shared<T>(this, std::forward<TArgs>(args)...));

			return static_cast<T*>(m_Subsystems.back().get());
		}

		template<EngineSystem T>
		T* GetSubsystem()
		{
			for (const auto& subsystem : m_Subsystems)
			{
				if (subsystem && typeid(T) == typeid(*subsystem))
					return static_cast<T*>(subsystem.get());
			}

			return nullptr;
		}

		bool OnInitialize()
		{
			bool success = true;
			for (auto& subsystem : m_Subsystems)
			{
				if (!subsystem->OnInitialize())
				{
					success = false;
					break;
				}
			}

			return success;
		}

		void OnTick()
		{
			for (auto& subsystem : m_Subsystems)
			{
				subsystem->OnTick();
			}
		}

		void OnShutdown()
		{
			// Shut down subsystems in reverse order, as to not mess with dependencies 
			for (auto& subsystem : m_Subsystems | std::views::reverse)
			{
				subsystem->OnShutdown();
			}
		}

		void OnEvent(Event& e)
		{
			for (auto& subsystem : m_Subsystems)
			{
				subsystem->OnEvent(e);
			}
		}

	private:
		std::vector<std::shared_ptr<Subsystem>> m_Subsystems;
	};
}
