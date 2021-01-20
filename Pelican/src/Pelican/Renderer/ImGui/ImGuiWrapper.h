#pragma once

#include <glm/vec2.hpp>
#include <vulkan/vulkan.h>

struct GLFWwindow;

namespace Pelican
{
	class VulkanDevice;
	class VulkanShader;

	struct ImGuiInitInfo
	{
		VkInstance instance;
		VkPhysicalDevice physicalDevice;
		VkDevice device;
		VkQueue queue;
		VkRenderPass renderPass;
	};

	class ImGuiWrapper
	{
	public:
		ImGuiWrapper();

		void Init(const ImGuiInitInfo& initInfo);
		void Cleanup();

		void NewFrame();
		void Render(VkCommandBuffer cmdBuffer);

	private:
		VkDevice m_Device{};
		VkDescriptorPool m_Pool{};
	};
}
