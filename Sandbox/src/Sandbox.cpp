// ReSharper disable once CppUnusedIncludeDirective

#if 0 // Re-enable this but on a pc that has VLD properly installed :)
#include <vld.h>
#endif

#include <Pelican.h>
#include <Pelican/Core/Entrypoint.h>

#include "SandboxLayer.h"

class Sandbox final : public Pelican::Application
{
public:
	Sandbox()
	{
		PushLayer(new SandboxLayer());
	}

	virtual ~Sandbox()
	{
	}

	void LoadScene(Pelican::Scene* pScene) override
	{
		using namespace Pelican;

		pScene->LoadFromFile("res/scenes/demoScene.json");
	}
};

Pelican::Application* Pelican::CreateApplication()
{
	return new Sandbox();
}
