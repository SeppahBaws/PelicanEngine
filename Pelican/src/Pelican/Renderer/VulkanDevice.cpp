#include "PelicanPCH.h"
#include "VulkanDevice.h"

#include <GLFW/glfw3.h>

#include "Pelican/Core/Application.h"
#include "VkInit.h"
#include "VulkanHelpers.h"

namespace Pelican
{
	VulkanDevice::VulkanDevice(VkInstance instance)
		: m_VulkanInstance(instance)
	{
		CreateSurface();
		PickPhysicalDevice();
		CreateLogicalDevice();
	}

	VulkanDevice::~VulkanDevice()
	{
		vkDestroyDevice(m_Device, nullptr);
		vkDestroySurfaceKHR(m_VulkanInstance, m_Surface, nullptr);
	}

	void VulkanDevice::WaitIdle()
	{
		vkDeviceWaitIdle(m_Device);
	}

	QueueFamilyIndices VulkanDevice::FindQueueFamilies()
	{
		ASSERT_MSG(m_PhysicalDevice != VK_NULL_HANDLE, "m_PhysicalDevice hasn't been initialized yet. Please pass in a valid physical device");

		return FindQueueFamilies(m_PhysicalDevice);
	}

	QueueFamilyIndices VulkanDevice::FindQueueFamilies(VkPhysicalDevice physicalDevice)
	{
		QueueFamilyIndices indices;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const auto& queueFamily : queueFamilies)
		{
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				indices.graphicsFamily = i;
			}

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, m_Surface, &presentSupport);
			if (presentSupport)
			{
				indices.presentFamily = i;
			}

			if (indices.IsComplete())
			{
				break;
			}

			i++;
		}

		return indices;
	}

	void VulkanDevice::CreateSurface()
	{
		VK_CHECK(glfwCreateWindowSurface(m_VulkanInstance, Application::Get().GetWindow()->GetGLFWWindow(), nullptr, &m_Surface));
	}

	void VulkanDevice::PickPhysicalDevice()
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(m_VulkanInstance, &deviceCount, nullptr);

		if (deviceCount == 0)
		{
			ASSERT_MSG(false, "failed to find GPUs with Vulkan support!");
		}

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(m_VulkanInstance, &deviceCount, devices.data());

		for (const auto& device : devices)
		{
			if (IsDeviceSuitable(device))
			{
				m_PhysicalDevice = device;
				break;
			}
		}

		if (m_PhysicalDevice == VK_NULL_HANDLE)
		{
			ASSERT_MSG(false, "failed to find a suitable GPU!");
		}
	}

	void VulkanDevice::CreateLogicalDevice()
	{
		QueueFamilyIndices indices = FindQueueFamilies(m_PhysicalDevice);

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(),indices.presentFamily.value() };

		float queuePriority = 1.0f;
		for (uint32_t queueFamily : uniqueQueueFamilies)
		{
			VkDeviceQueueCreateInfo queueCreateInfo = VkInit::DeviceQueueCreateInfo(queueFamily, 1, &queuePriority);
			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures{};
		deviceFeatures.samplerAnisotropy = VK_TRUE;

		VkDeviceCreateInfo createInfo = VkInit::DeviceCreateInfo(queueCreateInfos, &deviceFeatures, g_DeviceExtensions);

		VK_CHECK(vkCreateDevice(m_PhysicalDevice, &createInfo, nullptr, &m_Device));

		vkGetDeviceQueue(m_Device, indices.graphicsFamily.value(), 0, &m_GraphicsQueue);
		vkGetDeviceQueue(m_Device, indices.presentFamily.value(), 0, &m_PresentQueue);
	}


	bool VulkanDevice::IsDeviceSuitable(VkPhysicalDevice device)
	{
		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

		QueueFamilyIndices indices = FindQueueFamilies(device);

		bool extensionsSupported = VulkanHelpers::CheckDeviceExtensionSupport(device);

		bool swapChainAdequate = false;
		if (extensionsSupported)
		{
			SwapChainSupportDetails swapChainSupport = VulkanHelpers::QuerySwapChainSupport(device, m_Surface);
			swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		}

		VkPhysicalDeviceFeatures supportedFeatures;
		vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

		return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
			indices.IsComplete() && extensionsSupported && swapChainAdequate &&
			supportedFeatures.samplerAnisotropy;
	}
}
