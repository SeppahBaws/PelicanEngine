#pragma once

#include <entt.hpp>

#include "Scene.h"

namespace Pelican
{
	class SceneSerializer;

	class Entity final
	{
	public:
		Entity() = delete;

		template<typename T, typename... Args>
		T& AddComponent(Args&&... args)
		{
			ASSERT_MSG(!HasComponent<T>(), "Entity already has component!");
			T& component = m_pScene->m_Registry.emplace<T>(m_Entity, std::forward<Args>(args)...);
			return component;
		}

		template<typename T>
		T& AddComponent(const T& component)
		{
			ASSERT_MSG(!HasComponent<T>(), "Entity already has component!");
			T& c = m_pScene->m_Registry.emplace<T>(component);
			return c;
		}

		template<typename T>
		void RemoveComponent()
		{
			ASSERT_MSG(HasComponent<T>(), "Entity does not have component!");
			m_pScene->m_Registry.remove<T>(m_Entity);
		}

		template<typename T>
		bool HasComponent()
		{
			return m_pScene->m_Registry.all_of<T>(m_Entity);
		}

		template<typename T>
		T& GetComponent()
		{
			ASSERT_MSG(HasComponent<T>(), "Entity does not have component!");
			return m_pScene->m_Registry.get<T>(m_Entity);
		}

	private:
		// Scene is a friend, because it will create the entities for us.
		friend class Scene;
		friend class SceneSerializer;

		Entity(const entt::entity& e);
		Entity(const entt::entity& e, Scene* pScene);

	private:
		entt::entity m_Entity{ entt::null };
		Scene* m_pScene;
	};
}
