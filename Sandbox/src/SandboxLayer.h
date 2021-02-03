#pragma once
#include <Pelican.h>

class SandboxLayer : public Pelican::Layer
{
public:
	SandboxLayer();
	virtual ~SandboxLayer() = default;

	void OnAttach() override;
	void OnDetach() override;

	void OnUpdate() override;
	void OnImGuiRender() override;
	void OnEvent(Pelican::Event& e) override;

private:

};
