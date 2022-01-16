#pragma once

#include <glm/vec2.hpp>
#include <vulkan/vulkan.hpp>

struct GLFWwindow;

namespace Pelican
{
	struct ImGuiInitInfo
	{
		vk::Instance instance;
		vk::PhysicalDevice physicalDevice;
		vk::Device device;
		vk::Queue queue;
		vk::RenderPass renderPass;
	};

	class ImGuiWrapper
	{
	public:
		ImGuiWrapper();

		void Init(GLFWwindow* window, const ImGuiInitInfo& initInfo);
		void Cleanup();

		void NewFrame();
		void Render(vk::CommandBuffer cmdBuffer);

	private:
		vk::Device m_Device{};
		vk::DescriptorPool m_Pool{};
	};
}
