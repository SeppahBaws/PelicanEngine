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

		for (auto [entity, script] : m_Registry.view<ScriptComponent>().each())
		{
			script.OnUpdate();
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
