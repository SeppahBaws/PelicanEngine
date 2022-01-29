#pragma once
#include <vulkan/vulkan.hpp>

namespace Pelican
{
	class VulkanDevice;

	class VulkanBuffer
	{
	public:
		VulkanBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties);
		~VulkanBuffer();

		void CopyFromBuffer(const VulkanBuffer& buffer) const;

		[[nodiscard]] void* Map(vk::DeviceSize size) const;
		void Unmap() const;

		[[nodiscard]] vk::DeviceSize GetSize() const { return m_Size; }
		[[nodiscard]] vk::Buffer GetBuffer() const { return m_Buffer; }
		[[nodiscard]] vk::DeviceMemory GetMemory() const { return m_Memory; }

	private:
		vk::DeviceSize m_Size;
		vk::Buffer m_Buffer;
		vk::DeviceMemory m_Memory;
	};
}
