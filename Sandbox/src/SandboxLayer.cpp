#include "SandboxLayer.h"

#include <imgui.h>

SandboxLayer::SandboxLayer()
	: Layer("Sandbox")
{
}

void SandboxLayer::OnAttach()
{
}

void SandboxLayer::OnDetach()
{
}

void SandboxLayer::OnUpdate()
{
}

void SandboxLayer::OnImGuiRender()
{
	if (ImGui::Begin("Sandbox"))
	{
		ImGui::Text("Hello Sandbox!");
	}
	ImGui::End();
}

void SandboxLayer::OnEvent(Pelican::Event& /*e*/)
{
}
