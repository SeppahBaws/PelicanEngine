#pragma once

#include <optional>
#include <vulkan/vulkan.h>

namespace Pelican
{
	struct QueueFamilyIndices
	{
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool IsComplete() const
		{
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};

	class VulkanDevice final
	{
	public:
		VulkanDevice(VkInstance instance);
		~VulkanDevice();

		void WaitIdle();

		VkDevice GetDevice() const { return m_Device; }
		VkPhysicalDevice GetPhysicalDevice() const { return m_PhysicalDevice; }
		VkQueue GetGraphicsQueue() const { return m_GraphicsQueue; }
		VkQueue GetPresentQueue() const { return m_PresentQueue; }
		VkSurfaceKHR GetSurface() const { return m_Surface; }

		// Finds queue families for VulkanDevice's physical device.
		// !! Make sure to only call this function after the device has been initialized !!
		QueueFamilyIndices FindQueueFamilies();
		// Finds queue families for the given physical device
		QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice physicalDevice);

	private:
		void CreateSurface();
		void PickPhysicalDevice();
		void CreateLogicalDevice();

		bool IsDeviceSuitable(VkPhysicalDevice device);

	private:
		VkInstance m_VulkanInstance;

		VkSurfaceKHR m_Surface{};
		VkDevice m_Device{};
		VkPhysicalDevice m_PhysicalDevice{};
		VkQueue m_GraphicsQueue{};
		VkQueue m_PresentQueue{};
	};
}
