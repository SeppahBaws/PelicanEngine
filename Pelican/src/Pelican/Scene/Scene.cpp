#include "PelicanPCH.h"
#include "Scene.h"

#include <logtools.h>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

#include "Component.h"
#include "Entity.h"
#include "Serializer/SceneSerializer.h"

#include "Pelican/Core/Application.h"
#include "Pelican/Core/System/FileUtils.h"
#include "Pelican/Core/System/FileDialog.h"

#include "Pelican/Renderer/Camera.h"
#include "Pelican/Renderer/VulkanDebug.h"


namespace Pelican
{
	Scene::Scene()
	{
		m_pCamera = new Camera(120.0f,
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

		SceneSerializer::Deserialize(fileContent, this);
	}

	void Scene::SaveToFile(const std::string& file) const
	{
		std::string fileContent;

		SceneSerializer::Serialize(this, fileContent);

		const bool result = FileUtils::WriteFileSync(file, fileContent);
		if (!result)
		{
			Logger::LogError("Failed to write to \"%s\"", file.c_str());
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

		// TODO: Hacky solution, should be made better in the future
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

		VkDebugMarker::BeginRegion(VulkanRenderer::GetCurrentBuffer(), "Scene Render", glm::vec4(1.0f, 0.5f, 0.0f, 1.0f));

		for (auto [entity, transform, model] : view.each())
		{
			model.pModel->Draw();
		}

		VkDebugMarker::EndRegion(VulkanRenderer::GetCurrentBuffer());


		bool isOpen = true;
		// Debug UI
		if (ImGui::Begin("Scene debugger", &isOpen, ImGuiWindowFlags_MenuBar))
		{
			// Scene debugger menu bar
			if (ImGui::BeginMenuBar())
			{
				if (ImGui::BeginMenu("File"))
				{
					// TODO: implement this better, make the user choose a file. - Eventually should be moved to PelicanEd
					if (ImGui::MenuItem("Save scene file"))
					{
						Logger::LogDebug("Saving the current scene...");
						SaveToFile("res/scenes/demoScene.json");
					}
					if (ImGui::MenuItem("Load scene file"))
					{
						std::wstring filePath;
						if (FileDialog::OpenFileDialog(filePath))
						{
							Logger::LogDebug("Loading scene file \"%ls\"", filePath.c_str());
						}
					}
					ImGui::EndMenu();
				}
				ImGui::EndMenuBar();
			}

			ImGui::InputText("Scene Name", &m_Name);

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
		// (registry.destroy() on each entity might work, then we can move the memory free to the deconstructor of the components.)
		// Will have to test when VLD works again.
		for (auto [entity, model] : m_Registry.view<ModelComponent>().each())
		{
			delete model.pModel;
		}
	}
}
