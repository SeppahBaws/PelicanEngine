#pragma once

#include <optional>
#include <vulkan/vulkan.hpp>

namespace Pelican
{
	struct QueueFamilyIndices
	{
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		[[nodiscard]] bool IsComplete() const
		{
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};

	class VulkanDevice final
	{
	public:
		VulkanDevice(vk::Instance instance);
		~VulkanDevice();

		void WaitIdle();

		[[nodiscard]] vk::Device GetDevice() const { return m_Device.get(); }
		[[nodiscard]] vk::PhysicalDevice GetPhysicalDevice() const { return m_PhysicalDevice; }
		[[nodiscard]] vk::Queue GetGraphicsQueue() const { return m_GraphicsQueue; }
		[[nodiscard]] vk::Queue GetPresentQueue() const { return m_PresentQueue; }
		[[nodiscard]] vk::SurfaceKHR GetSurface() const { return m_Surface; }

		// Finds queue families for VulkanDevice's physical device.
		// !! Make sure to only call this function after the device has been initialized !!
		[[nodiscard]] QueueFamilyIndices FindQueueFamilies() const;
		// Finds queue families for the given physical device
		[[nodiscard]] QueueFamilyIndices FindQueueFamilies(vk::PhysicalDevice physicalDevice) const;

	private:
		void CreateSurface();
		void PickPhysicalDevice();
		void CreateLogicalDevice();

		bool IsDeviceSuitable(vk::PhysicalDevice device) const;

	private:
		vk::Instance m_Instance;

		vk::SurfaceKHR m_Surface{};
		vk::UniqueDevice m_Device{};
		vk::PhysicalDevice m_PhysicalDevice{};
		vk::Queue m_GraphicsQueue{};
		vk::Queue m_PresentQueue{};
	};
}
