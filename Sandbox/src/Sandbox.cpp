#ifdef SANDBOX_USE_VLD
// ReSharper disable once CppUnusedIncludeDirective
#include <vld.h>
#endif

#include <Pelican.h>
#include <Pelican/Core/Entrypoint.h>

class Sandbox final : public Pelican::Application
{
public:
	explicit Sandbox(const Pelican::ApplicationSpecification& specification)
		: Application(specification)
	{}

	void LoadScene(Pelican::Scene* pScene) override
	{
		using namespace Pelican;

		pScene->LoadFromFile("res/scenes/demoScene.json");
	}
};

Pelican::Application* Pelican::CreateApplication()
{
	const Pelican::ApplicationSpecification applicationSpecification = {
		.name = "Sandbox",
		.windowWidth = 1920,
		.windowHeight = 1080,
		.fullscreen = false,
		.vsync = true,
		.resizable = true,
		.startMaximized = false
	};
	return new Sandbox(applicationSpecification);
}
