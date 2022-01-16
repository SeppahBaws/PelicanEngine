#include "PelicanPCH.h"
#include "VulkanDevice.h"

#include <GLFW/glfw3.h>

#include "Pelican/Core/Application.h"
#include "VkInit.h"
#include "VulkanHelpers.h"

namespace Pelican
{
	VulkanDevice::VulkanDevice(Context* pContext, vk::Instance instance)
		: m_pContext(pContext)
		, m_Instance(instance)
	{
		CreateSurface();
		PickPhysicalDevice();
		CreateLogicalDevice();
	}

	VulkanDevice::~VulkanDevice()
	{
		vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
	}

	void VulkanDevice::WaitIdle()
	{
		m_Device->waitIdle();
	}

	QueueFamilyIndices VulkanDevice::FindQueueFamilies() const
	{
		if (!m_PhysicalDevice)
		{
			throw std::runtime_error("m_PhysicalDevice hasn't been initialized yet. Please pass in a valid physical device");
		}

		return FindQueueFamilies(m_PhysicalDevice);
	}

	QueueFamilyIndices VulkanDevice::FindQueueFamilies(vk::PhysicalDevice physicalDevice) const
	{
		QueueFamilyIndices indices;

		uint32_t queueFamilyCount = 0;
		physicalDevice.getQueueFamilyProperties(&queueFamilyCount, nullptr);

		std::vector<vk::QueueFamilyProperties> queueFamilies(queueFamilyCount);
		physicalDevice.getQueueFamilyProperties(&queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const auto& queueFamily : queueFamilies)
		{
			if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)
			{
				indices.graphicsFamily = i;
			}

			vk::Bool32 presentSupport = false;
			const vk::Result result = physicalDevice.getSurfaceSupportKHR(i, m_Surface, &presentSupport);
			if (result != vk::Result::eSuccess)
			{
				throw std::runtime_error("Failed to get physical device surface support!");
			}

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
		GLFWwindow* window = m_pContext->GetSubsystem<Window>()->GetGLFWWindow();
		VkSurfaceKHR rawSurface;
		VK_CHECK(glfwCreateWindowSurface(m_Instance, window, nullptr, &rawSurface));

		m_Surface = rawSurface;
	}

	void VulkanDevice::PickPhysicalDevice()
	{
		std::vector<vk::PhysicalDevice> devices = m_Instance.enumeratePhysicalDevices();
		if (devices.empty())
		{
			throw std::runtime_error("Failed to find GPUs with Vulkan support!");
		}

		for (const vk::PhysicalDevice& device : devices)
		{
			if (IsDeviceSuitable(device))
			{
				m_PhysicalDevice = device;
				break;
			}
		}

		if (!m_PhysicalDevice)
		{
			throw std::runtime_error("Failed to find a suitable GPU!");
		}
	}

	void VulkanDevice::CreateLogicalDevice()
	{
		QueueFamilyIndices indices = FindQueueFamilies(m_PhysicalDevice);
		float queuePriority = 1.0f;
		vk::DeviceQueueCreateInfo queueCreateInfo = vk::DeviceQueueCreateInfo(
			{},
			indices.graphicsFamily.value(),
			1,
			&queuePriority
		);

		vk::PhysicalDeviceFeatures deviceFeatures = vk::PhysicalDeviceFeatures();
		deviceFeatures.samplerAnisotropy = true;
		deviceFeatures.fillModeNonSolid = true;

		if (PELICAN_VALIDATE)
		{
			g_DeviceExtensions.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
		}

		const vk::DeviceCreateInfo createInfo = vk::DeviceCreateInfo(
			{},
			1, &queueCreateInfo,
			0, nullptr,
			static_cast<uint32_t>(g_DeviceExtensions.size()),
			g_DeviceExtensions.data(),
			&deviceFeatures
		);

		try
		{
			m_Device = m_PhysicalDevice.createDeviceUnique(createInfo);
		}
		catch (vk::SystemError& e)
		{
			throw std::runtime_error("Failed to create logical device: "s + e.what());
		}

		m_GraphicsQueue = m_Device->getQueue(indices.graphicsFamily.value(), 0);
		m_PresentQueue = m_Device->getQueue(indices.presentFamily.value(), 0);
	}

	bool VulkanDevice::IsDeviceSuitable(vk::PhysicalDevice device) const
	{
		vk::PhysicalDeviceProperties deviceProperties;
		vk::PhysicalDeviceFeatures deviceFeatures;
		device.getProperties(&deviceProperties);
		device.getFeatures(&deviceFeatures);
		
		QueueFamilyIndices indices = FindQueueFamilies(device);

		bool extensionsSupported = VulkanHelpers::CheckDeviceExtensionSupport(device);

		bool swapChainAdequate = false;
		if (extensionsSupported)
		{
			SwapChainSupportDetails swapChainSupport = VulkanHelpers::QuerySwapChainSupport(device, m_Surface);
			swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		}

		vk::PhysicalDeviceFeatures supportedFeatures;
		device.getFeatures(&supportedFeatures);

		return deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu &&
			indices.IsComplete() && extensionsSupported && swapChainAdequate &&
			supportedFeatures.samplerAnisotropy;
	}
}
