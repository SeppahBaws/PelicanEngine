#include "PelicanPCH.h"
#include "Scene.h"

#include <logtools.h>
#include <imgui.h>
#include <json.hpp>

#pragma warning(push, 0)
#include <glm/gtc/type_ptr.hpp>
#pragma warning(pop)

#include "Component.h"
#include "Entity.h"
#include "Pelican/Core/Application.h"
#include "Pelican/Core/System/FileUtils.h"

#include "Pelican/Renderer/Camera.h"

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
}

namespace Pelican
{
	Scene::Scene()
	{
		m_pCamera = m_pCamera = new Camera(120.0f,
			static_cast<float>(Application::Get().GetWindow()->GetParams().width),
			static_cast<float>(Application::Get().GetWindow()->GetParams().height),
			0.1f, 1000.0f);
		Application::Get().GetRenderer().SetCamera(m_pCamera);

		Initialize();
	}

	Scene::~Scene()
	{
		Cleanup();
	}

	void Scene::LoadFromFile(const std::string& file)
	{
		std::string fileContent;

		const bool result = FileUtils::ReadFileSync(file, fileContent);
		if (!result)
		{
			Logger::LogError("Failed to read \"%s\"!", file.c_str());
			return;
		}

		using namespace nlohmann;

		basic_json jsonScene = json::parse(fileContent);

		const auto name = jsonScene["name"];
		const auto jsonEntities = jsonScene["entities"];

		if (!name.is_string())
		{
			throw std::exception("Invalid scene name");
		}

		if (!jsonEntities.is_array())
		{
			throw std::exception("Invalid scene entities");
		}

		// Get the scene name
		name.get_to(m_Name);

		for (auto jsonEntity : jsonEntities)
		{
			const auto id = jsonEntity["id"];
			const auto jsonComponents = jsonEntity["components"];

			Entity e = m_Registry.create();
			e.m_pScene = this;

			for (auto jsonComponent : jsonComponents)
			{
				const auto componentType = jsonComponent["type"];

				// TODO: get rid of this garbage
				if (componentType == "TagComponent")
				{
					// Parse tag component
					if (!jsonComponent["name"].is_string())
						throw std::exception("Tag component doesn't have a name!");

					std::string tagName{};
					jsonComponent["name"].get_to(tagName);
					e.AddComponent<TagComponent>(tagName);
				}
				else if (componentType == "TransformComponent")
				{
					// Parse transform component
					if (!jsonComponent["position"].is_array())
						throw std::exception("Transform component doesn't have a position!");

					if (!jsonComponent["rotation"].is_array())
						throw std::exception("Transform component doesn't have a rotation!");

					if (!jsonComponent["scale"].is_array())
						throw std::exception("Transform component doesn't have a scale!");

					glm::vec3 position;
					glm::vec3 rotation;
					glm::vec3 scale;

					jsonComponent["position"].get_to(position);
					jsonComponent["rotation"].get_to(rotation);
					jsonComponent["scale"].get_to(scale);

					e.AddComponent<TransformComponent>(position, rotation, scale);
				}
				else if (componentType == "ModelComponent")
				{
					// Parse model component
					if (!jsonComponent["assetPath"].is_string())
						throw std::exception("Model component doesn't have an asset path!");

					std::string assetPath;
					jsonComponent["assetPath"].get_to(assetPath);
					e.AddComponent<ModelComponent>(new GltfModel(assetPath));
				}
			}
		}
	}

	Entity Scene::CreateEntity(const std::string& name)
	{
		const auto entity = m_Registry.create();
		Entity e(entity);
		e.m_pScene = this;

		m_Registry.emplace<TagComponent>(entity, std::string(name));

		return e;
	}

	void Scene::DestroyEntity(Entity& entity)
	{
		m_Registry.destroy(entity.m_Entity);
	}

	void Scene::Initialize()
	{
	}

	void Scene::Update()
	{
		m_pCamera->Update();

		// Hacky solution, should be made better in the future
		for (auto [entity, transform, model] : m_Registry.view<TransformComponent, ModelComponent>().each())
		{
			glm::mat4 m = transform.GetTransform();
			glm::mat4 v = m_pCamera->GetView();
			glm::mat4 p = m_pCamera->GetProjection();
			p[1][1] *= -1;

			model.pModel->Update(m, v, p);
		}
	}

	void Scene::Draw()
	{
		auto view = m_Registry.view<TransformComponent, ModelComponent>();

		for (auto [entity, transform, model] : view.each())
		{
			model.pModel->Draw();
		}


		// Debug UI
		if (ImGui::Begin("Scene debugger"))
		{
			const auto entities = m_Registry.view<TagComponent>();

			ImGui::Text("%i entities in scene:", static_cast<int>(entities.size()));
			ImGui::Spacing();

			for (auto [entity, tag] : entities.each())
			{
				ImGui::Text("%s", tag.name.c_str());
			}
		}
		ImGui::End();

		for (auto [entity, tag, transform] : m_Registry.view<TagComponent, TransformComponent>().each())
		{
			if (ImGui::Begin(std::string("Entity debugger##" + tag.name).c_str()))
			{
				ImGui::Text("Debug for: %s", tag.name.c_str());
				ImGui::Spacing();
				ImGui::InputFloat3("position", reinterpret_cast<float*>(&transform.position));
				ImGui::InputFloat3("rotation", reinterpret_cast<float*>(&transform.rotation));
				ImGui::InputFloat3("scale", reinterpret_cast<float*>(&transform.scale));
			}
			ImGui::End();
		}
	}

	void Scene::Cleanup()
	{
		// TEMP - camera should be in a CameraComponent
		delete m_pCamera;

		// TODO: figure out a way to make this easier.
		for (auto [entity, model] : m_Registry.view<ModelComponent>().each())
		{
			delete model.pModel;
		}
	}
}
