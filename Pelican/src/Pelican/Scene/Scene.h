#pragma once

#include <entt.hpp>

#include "Pelican/Renderer/UniformData.h"

namespace Pelican
{
	class Camera;
	class Entity;
	class SceneSerializer;

	class Scene
	{
	public:
		Scene();
		~Scene();

		void LoadFromFile(const std::string& file);
		void SaveToFile(const std::string& file) const;

		Entity CreateEntity(const std::string& name = "new entity");
		void DestroyEntity(Entity& entity);

		const DirectionalLight& GetDirectionalLight() { return m_DirectionalLight; }

		void SetName(const std::string& name) { m_Name = name; }
		std::string GetName() const { return m_Name; }

		void Initialize();
		void Update();
		void Draw();
		void Cleanup();

	private:
		// We need access to the registry to add components.
		friend class Entity;
		friend class SceneSerializer;

		entt::registry m_Registry;

		std::string m_Name{};
		DirectionalLight m_DirectionalLight;

		// TEMP
		Camera* m_pCamera;
	};
}
