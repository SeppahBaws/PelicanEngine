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

		void LoadFromFile(const std::string& file);

		Entity CreateEntity(const std::string& name = "new entity");
		void DestroyEntity(Entity& entity);

		void SetName(const std::string& name) { m_Name = name; }
		std::string GetName() const { return m_Name; }

		void Initialize();
		void Update();
		void Draw();
		void Cleanup();

	private:
		// We need access to the registry to add components.
		friend class Entity;

		entt::registry m_Registry;

		std::string m_Name{};

		// TEMP
		Camera* m_pCamera;
	};
}
