#include "PelicanPCH.h"
#include "SceneSerializer.h"

#include <json.hpp>
#include <entt.hpp>

#include "../Entity.h"
#include "../Component.h"
#include "../Scene.h"

#include "ald_serializer.h"
#include "Pelican/Assets/AssetManager.h"

namespace Pelican
{
	void SceneSerializer::Serialize(const Scene* pScene, std::string& serialized)
	{
		using namespace nlohmann;

		json jsonScene = json::object();
		jsonScene["name"] = pScene->m_Name;

		json lighting = json::object();
		lighting["directionalLight"] = pScene->m_DirectionalLight;
		lighting["pointLight"] = pScene->m_PointLight;

		json environment = json::object();
		environment["skybox"] = pScene->m_Skybox->GetAssetPath();
		environment["radiance"] = pScene->m_Radiance->GetAssetPath();
		environment["irradiance"] = pScene->m_Irradiance->GetAssetPath();
		lighting["environmentMap"] = environment;

		jsonScene["lighting"] = lighting;

		json jEntities = json::array();

		pScene->m_Registry.each([&](auto entity)
		{
			json jEntity = json::object();
			Entity e = Entity{ entity };
			e.m_pScene = const_cast<Scene*>(pScene);

			json jComponents = json::array();

			if (e.HasComponent<TagComponent>())
			{
				TagComponent c = e.GetComponent<TagComponent>();
				const json tag = c;
				jComponents.push_back(tag);
			}
			if (e.HasComponent<TransformComponent>())
			{
				TransformComponent c = e.GetComponent<TransformComponent>();
				const json j = c;
				jComponents.push_back(j);
			}
			if (e.HasComponent<ModelComponent>())
			{
				ModelComponent c = e.GetComponent<ModelComponent>();
				const json j = c;
				jComponents.push_back(c);
			}

			jEntity["components"] = jComponents;
			jEntities.push_back(jEntity);
		});

		jsonScene["entities"] = jEntities;

		serialized = jsonScene.dump(2);
	}

	void SceneSerializer::Deserialize(const std::string& serialized, Scene* pScene)
	{
		using namespace nlohmann;

		json jsonScene = json::parse(serialized);

		const json jName = jsonScene["name"];
		const json jLighting = jsonScene["lighting"];
		const json jEntities = jsonScene["entities"];

		if (!jName.is_string())
		{
			throw std::exception("Invalid scene name");
		}

		if (!jEntities.is_array())
		{
			throw std::exception("Invalid scene entities");
		}

		// Get the scene name
		jName.get_to(pScene->m_Name);

		jLighting["directionalLight"].get_to(pScene->m_DirectionalLight);
		jLighting["pointLight"].get_to(pScene->m_PointLight);

		const json jEnv = jLighting["environmentMap"];
		pScene->m_Skybox = AssetManager::GetInstance().LoadTexture(jEnv["skybox"], VulkanTexture::TextureMode::Cubemap);
		pScene->m_Radiance = AssetManager::GetInstance().LoadTexture(jEnv["radiance"], VulkanTexture::TextureMode::Cubemap);
		pScene->m_Irradiance = AssetManager::GetInstance().LoadTexture(jEnv["irradiance"], VulkanTexture::TextureMode::Cubemap);

		for (json jEntity : jEntities)
		{
			const json id = jEntity["id"];
			const json jComponents = jEntity["components"];

			Entity e = pScene->m_Registry.create();
			e.m_pScene = pScene;

			for (auto jComponent : jComponents)
			{
				const json jComponentType = jComponent["type"];

				// TODO: get rid of this garbage
				if (jComponentType == "TagComponent")
				{
					// Parse tag component
					if (!jComponent["name"].is_string())
						throw std::exception("Tag component doesn't have a name!");

					std::string tagName{};
					jComponent["name"].get_to(tagName);
					e.AddComponent<TagComponent>(tagName);
				}
				else if (jComponentType == "TransformComponent")
				{
					// Parse transform component
					if (!jComponent["position"].is_array())
						throw std::exception("Transform component doesn't have a position!");

					if (!jComponent["rotation"].is_array())
						throw std::exception("Transform component doesn't have a rotation!");

					if (!jComponent["scale"].is_array())
						throw std::exception("Transform component doesn't have a scale!");

					glm::vec3 position, rotation, scale;

					jComponent["position"].get_to(position);
					jComponent["rotation"].get_to(rotation);
					jComponent["scale"].get_to(scale);

					e.AddComponent<TransformComponent>(position, rotation, scale);
				}
				else if (jComponentType == "ModelComponent")
				{
					// Parse model component
					if (!jComponent["assetPath"].is_string())
						throw std::exception("Model component doesn't have an asset path!");

					std::string assetPath;
					jComponent["assetPath"].get_to(assetPath);
					e.AddComponent<ModelComponent>(new GltfModel(assetPath));
				}
			}
		}
	}
}
