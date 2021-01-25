// ReSharper disable once CppUnusedIncludeDirective
#include <vld.h>
#include <Pelican.h>

class Sandbox final : public Pelican::Application
{
public:
	Sandbox() = default;
	virtual ~Sandbox() = default;

	void LoadScene(Pelican::Scene* pScene) override
	{
		using namespace Pelican;

		Entity light = pScene->CreateEntity("Light");
		light.AddComponent<TransformComponent>(glm::vec3{ 0.0f, 5.0f, 0.0f }, glm::vec3{ -90.0f, 0.0f, 0.0f }, glm::vec3{ 0.1f });
		light.AddComponent<ModelComponent>(new GltfModel("res/models/tactical_flashlight/scene.gltf"));

		Entity light2 = pScene->CreateEntity("Light 2");
		light2.AddComponent<TransformComponent>(glm::vec3{ 3.0f, 3.0f, 3.0f }, glm::vec3{ -90.0f, 0.0f, 0.0f }, glm::vec3{ 0.1f });
		light2.AddComponent<ModelComponent>(new GltfModel("res/models/tactical_flashlight/scene.gltf"));

		Entity car = pScene->CreateEntity("Car");
		car.AddComponent<TransformComponent>(glm::vec3{ 0.0f, 0.0f, 0.0f }, glm::vec3{ -90.0f, 0.0f, .0f }, glm::vec3{ 0.01f });
		car.AddComponent<ModelComponent>(new GltfModel("res/models/pony_cartoon/scene.gltf"));
	}
};

Pelican::Application* Pelican::CreateApplication()
{
	return new Sandbox();
	// return new Pelican::Application();
}
