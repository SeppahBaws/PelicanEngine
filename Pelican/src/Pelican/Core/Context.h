#pragma once

#include <ranges>

#include "Subsystem.h"

namespace Pelican
{
	class Context final
	{
	public:
		Context() = default;

		template<typename T, typename = std::enable_if_t<std::is_base_of_v<Subsystem, T>>, class... TArgs>
		void AddSubsystem(TArgs&&... args)
		{
			m_Subsystems.emplace_back(std::make_shared<T>(this, std::forward<TArgs>(args)...));
		}

		template<typename T, typename = std::enable_if_t<std::is_base_of_v<Subsystem, T>>>
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

	private:
		std::vector<std::shared_ptr<Subsystem>> m_Subsystems;
	};
}
