#include "PelicanPCH.h"
#include "Scene.h"

#include <logtools.h>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

#include "Component.h"
#include "Entity.h"
#include "Serializer/SceneSerializer.h"

#include "Pelican/Assets/AssetManager.h"

#include "Pelican/Core/Application.h"
#include "Pelican/Core/Time.h"
#include "Pelican/IO/FileUtils.h"
#include "Pelican/IO/FileDialog.h"

#include "Pelican/Renderer/Camera.h"
#include "Pelican/Renderer/UniformData.h"
#include "Pelican/Renderer/VulkanDebug.h"


namespace Pelican
{
	Scene::Scene(Context* pContext)
		: Subsystem(pContext)
	{
	}

	bool Scene::OnInitialize()
	{
		Window* pWindow = m_pContext->GetSubsystem<Window>();
		VulkanRenderer* pRenderer = m_pContext->GetSubsystem<VulkanRenderer>();
		m_pCamera = new Camera(m_pContext, 120.0f,
			static_cast<float>(pWindow->GetSpecification().width),
			static_cast<float>(pWindow->GetSpecification().height),
			0.1f, 1000.0f);
		pRenderer->SetCamera(m_pCamera);

		return true;
	}

	void Scene::OnTick()
	{
		m_pCamera->Update();

		if (m_AnimateLight)
			m_PointLight.position = glm::vec3(cos(Time::GetTotalTime()) * 10.0f, 30.0f, sin(Time::GetTotalTime()) * 10.0f);

		for (auto [entity, transform, model] : m_Registry.view<TransformComponent, ModelComponent>().each())
		{
			glm::mat4 m = transform.GetTransform();
			glm::mat4 v = m_pCamera->GetView();
			glm::mat4 p = m_pCamera->GetProjection();
			p[1][1] *= -1;

			model.pModel->UpdateDrawData(m, v, p);
		}

		Draw();
	}

	void Scene::OnShutdown()
	{
		// TODO: figure out a way to make this easier.
		// (registry.destroy() on each entity might work, then we can move the memory free to the deconstructor of the components.)
		// Will have to test when VLD works again.
		for (auto [entity, model] : m_Registry.view<ModelComponent>().each())
		{
			delete model.pModel;
		}

		AssetManager::GetInstance().UnloadTexture(m_Skybox);
		AssetManager::GetInstance().UnloadTexture(m_Radiance);
		AssetManager::GetInstance().UnloadTexture(m_Irradiance);

		delete m_pCamera;
	}

	void Scene::LoadFromFile(const std::string& file)
	{
		std::string fileContent;

		const bool result = IO::ReadFileSync(file, fileContent);
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

		const bool result = IO::WriteFileSync(file, fileContent);
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

	// Temp, will be moved to its own EditorPanel in the future.
	static std::optional<entt::entity> selectedEntity{};

	void Scene::Draw()
	{
		auto view = m_Registry.view<TransformComponent, ModelComponent>();

		vk::CommandBuffer cmd = VulkanRenderer::GetCurrentBuffer();

		//
		// Scene Render
		//
		VkDebugMarker::BeginRegion(cmd, "Scene Render", glm::vec4(1.0f, 0.5f, 0.0f, 1.0f));

		cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, VulkanRenderer::GetCurrentPipeline());

		// Bind push constants
		CameraPushConst pushConst;
		pushConst.eyePos = m_pCamera->GetPosition();

		cmd.pushConstants(VulkanRenderer::GetPipelineLayout(), vk::ShaderStageFlagBits::eFragment, 0, sizeof(CameraPushConst), &pushConst);

		// Draw meshes
		for (auto [entity, transform, model] : view.each())
		{
			model.pModel->Draw(m_FrameIdx);
		}

		VkDebugMarker::EndRegion(cmd);


		// TODO: move this out to an editor or so...
#pragma region Debug UI
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
						if (IO::FileDialog::OpenFileDialog(filePath))
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
				ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
				if (selectedEntity == entity)
					flags |= ImGuiTreeNodeFlags_Selected;

				// if (entity.has_children())
				flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

				ImGui::TreeNodeEx(tag.name.c_str(), flags);
				if (ImGui::IsItemClicked())
					selectedEntity = entity;
			}
		}
		ImGui::End();

		if (ImGui::Begin("Selected entity"))
		{
			auto draw = [&]()
			{
				if (!selectedEntity)
					return;

				auto [tag, transform, model] = m_Registry.get<TagComponent, TransformComponent, ModelComponent>(*selectedEntity);

				ImGui::InputText("Name", &tag.name);

				ImGui::Spacing();
				if (ImGui::CollapsingHeader("Transform Component", ImGuiTreeNodeFlags_DefaultOpen))
				{
					ImGui::InputFloat3("position", reinterpret_cast<float*>(&transform.position));
					ImGui::InputFloat3("rotation", reinterpret_cast<float*>(&transform.rotation));
					ImGui::InputFloat3("scale", reinterpret_cast<float*>(&transform.scale));
				}

				ImGui::Spacing();

				if (ImGui::CollapsingHeader("Model Component", ImGuiTreeNodeFlags_DefaultOpen))
				{
					ImGui::LabelText("Asset path", model.pModel->GetAssetPath().c_str());
				}
			};

			draw();
		}
		ImGui::End();

		if (ImGui::Begin("Light Settings"))
		{
			if (ImGui::CollapsingHeader("Directional Light"))
			{
				ImGui::DragFloat3("direction", reinterpret_cast<float*>(&m_DirectionalLight.direction), 0.01f, -1.0f, 1.0f, "%.3f");
				ImGui::ColorEdit3("light color", reinterpret_cast<float*>(&m_DirectionalLight.lightColor));
				ImGui::ColorEdit3("ambient color", reinterpret_cast<float*>(&m_DirectionalLight.ambientColor));
			}

			if (ImGui::CollapsingHeader("Point Light"))
			{
				ImGui::Checkbox("Animate Light", &m_AnimateLight);
				ImGui::InputFloat3("Position", reinterpret_cast<float*>(&m_PointLight.position));
				ImGui::InputFloat3("Diffuse", reinterpret_cast<float*>(&m_PointLight.diffuse));
			}
		}
		ImGui::End();

		AssetManager::GetInstance().DebugDraw();
#pragma endregion

		m_FrameIdx = (m_FrameIdx + 1) % 3;
	}
}
