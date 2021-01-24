#include "PelicanPCH.h"
#include "Scene.h"

#include <logtools.h>
#include <imgui.h>

#include "Component.h"
#include "Entity.h"
#include "Pelican/Application.h"

#include "Pelican/Renderer/Camera.h"

namespace Pelican
{
	struct Position
	{
		float x, y, z;
	};

	Scene::Scene()
	{
		Initialize();
	}

	Scene::~Scene()
	{
		Cleanup();
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
		// TEMP
		m_pCamera = m_pCamera = new Camera(120.0f,
			static_cast<float>(Application::Get().GetWindow()->GetParams().width),
			static_cast<float>(Application::Get().GetWindow()->GetParams().height),
			0.1f, 1000.0f);
		Application::Get().GetRenderer().SetCamera(m_pCamera);

		Entity light = CreateEntity("Light");
		light.AddComponent<TransformComponent>(glm::vec3{ 0.0f, 5.0f, 0.0f }, glm::vec3{ glm::radians(-90.0f), 0.0f, 0.0f }, glm::vec3{ 0.1f });
		light.AddComponent<ModelComponent>(new GltfModel("res/models/tactical_flashlight/scene.gltf"));

		Entity car = CreateEntity("Car");
		car.AddComponent<TransformComponent>(glm::vec3{ 0.0f, 0.0f, 0.0f }, glm::vec3{ glm::radians(-90.0f), 0.0f, 0.0f }, glm::vec3{ 0.01f });
		car.AddComponent<ModelComponent>(new GltfModel("res/models/pony_cartoon/scene.gltf"));
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
