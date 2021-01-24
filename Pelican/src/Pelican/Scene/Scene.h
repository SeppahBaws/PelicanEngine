#pragma once

#include <entt.hpp>


namespace Pelican
{
	class Camera;
	class Entity;

	class Scene
	{
	public:
		Scene();
		~Scene();

		Entity CreateEntity(const std::string& name = "new entity");
		void DestroyEntity(Entity& entity);

		void Initialize();
		void Update();
		void Draw();
		void Cleanup();

	private:
		// We need access to the registry to add components.
		friend class Entity;

		entt::registry m_Registry;

		// TEMP
		Camera* m_pCamera;
	};
}
