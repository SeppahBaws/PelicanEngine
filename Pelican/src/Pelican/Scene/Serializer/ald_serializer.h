#pragma once

#include <json.hpp>

#pragma warning(push, 0)
#include <glm/vec3.hpp>
#include <glm/gtc/type_ptr.hpp>
#pragma warning(pop)

#include "../Component.h"

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

	template<>
	struct adl_serializer<Pelican::DirectionalLight>
	{
		static void to_json(json& j, const Pelican::DirectionalLight& t)
		{
			j = json::object();
			j["direction"] = t.direction;
			j["lightColor"] = t.lightColor;
			j["ambientColor"] = t.ambientColor;
		}

		static void from_json(const json& j, Pelican::DirectionalLight& t)
		{
			t.direction = j["direction"];
			t.lightColor = j["lightColor"];
			t.ambientColor = j["ambientColor"];
		}
	};

	template<>
	struct adl_serializer<Pelican::PointLight>
	{
		static void to_json(json& j, const Pelican::PointLight& l)
		{
			j = json::object();
			j["position"] = l.position;
			j["diffuse"] = l.diffuse;
		}

		static void from_json(const json& j, Pelican::PointLight& l)
		{
			l.position = j["position"];
			l.diffuse = j["diffuse"];
		}
	};
}
