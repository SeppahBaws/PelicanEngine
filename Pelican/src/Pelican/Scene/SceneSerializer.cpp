﻿#include "PelicanPCH.h"
#include "SceneSerializer.h"

#include <json.hpp>
#include <entt.hpp>

#pragma warning(push, 0)
#include <glm/vec3.hpp>
#include <glm/gtc/type_ptr.hpp>
#pragma warning(pop)

#include "Entity.h"
#include "Component.h"
#include "Scene.h"

namespace nlohmann
{
	template<>
	struct adl_serializer<glm::vec3>
	{
		static void to_json(json& j, const glm::vec3& v)
		{
			j = json::array({ v.x, v.y, v.z });
		}

		static void from_json(const json& j, glm::vec3& v)
		{
			std::vector<float> values;
			j.get_to(values);
			v = glm::make_vec3(values.data());
		}
	};

	template<>
	struct adl_serializer<Pelican::TagComponent>
	{
		static void to_json(json& j, const Pelican::TagComponent& t)
		{
			j = json::object();
			j["type"] = "TagComponent";
			j["name"] = t.name;
		}

		static void from_json(const json& j, Pelican::TagComponent& t)
		{
			if (j["type"] != "TagComponent")
				return;

			t.name = j["name"];
		}
	};

	template<>
	struct adl_serializer<Pelican::TransformComponent>
	{
		static void to_json(json& j, const Pelican::TransformComponent& t)
		{
			j = json::object();
			j["type"] = "TransformComponent";
			j["position"] = t.position;
			j["rotation"] = t.rotation;
			j["scale"] = t.scale;
		}

		static void from_json(const json& j, Pelican::TransformComponent& t)
		{
			if (j["type"] != "TransformComponent")
				return;

			t.position = j["position"];
			t.rotation = j["rotation"];
			t.scale = j["scale"];
		}
	};

	template<>
	struct adl_serializer<Pelican::ModelComponent>
	{
		static void to_json(json& j, const Pelican::ModelComponent& t)
		{
			j = json::object();
			j["type"] = "ModelComponent";
			j["assetPath"] = t.pModel->GetAssetPath();
		}

		static void from_json(const json& j, Pelican::ModelComponent& t)
		{
			if (j["type"] != "TagComponent")
				return;

			const std::string assetPath = j["assetPath"];
			t.pModel = new Pelican::GltfModel(assetPath);
		}
	};
}

namespace Pelican
{
	void SceneSerializer::Serialize(const Scene* pScene, std::string& serialized)
	{
		using namespace nlohmann;

		json jsonScene = json::object();
		jsonScene["name"] = pScene->m_Name;
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

					glm::vec3 position;
					glm::vec3 rotation;
					glm::vec3 scale;

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
