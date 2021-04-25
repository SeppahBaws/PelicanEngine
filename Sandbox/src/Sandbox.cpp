#ifdef SANDBOX_USE_VLD
// ReSharper disable once CppUnusedIncludeDirective
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
