#include "PelicanPCH.h"
#include "VulkanBuffer.h"

#include "VulkanHelpers.h"
#include "Pelican/Renderer/VulkanRenderer.h"

namespace Pelican
{
	VulkanBuffer::VulkanBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties)
		: m_Size(size)
	{
		vk::BufferCreateInfo bufferInfo = {};
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = vk::SharingMode::eExclusive;

		try
		{
			m_Buffer = VulkanRenderer::GetDevice().createBuffer(bufferInfo);
		}
		catch (vk::SystemError& e)
		{
			throw std::runtime_error("Failed to create buffer: "s + e.what());
		}

		const vk::MemoryRequirements memRequirements = VulkanRenderer::GetDevice().getBufferMemoryRequirements(m_Buffer);

		const vk::MemoryAllocateInfo allocInfo(memRequirements.size, VulkanHelpers::FindMemoryType(memRequirements.memoryTypeBits, properties));

		try
		{
			m_Memory = VulkanRenderer::GetDevice().allocateMemory(allocInfo);
		}
		catch (vk::SystemError& e)
		{
			throw std::runtime_error("Failed to allocate buffer memory: "s + e.what());
		}

		try
		{
			VulkanRenderer::GetDevice().bindBufferMemory(m_Buffer, m_Memory, 0);
		}
		catch (vk::SystemError& e)
		{
			throw std::runtime_error("Failed to bind buffer memory: "s + e.what());
		}
	}

	VulkanBuffer::~VulkanBuffer()
	{
		const vk::Device device = VulkanRenderer::GetDevice();
		device.destroyBuffer(m_Buffer);
		device.freeMemory(m_Memory);
	}

	void VulkanBuffer::CopyFromBuffer(const VulkanBuffer& buffer) const
	{
		const vk::CommandBuffer cmd = VulkanHelpers::BeginSingleTimeCommands();

		vk::BufferCopy copy = { {}, {}, buffer.m_Size };
		cmd.copyBuffer(buffer.m_Buffer, m_Buffer, { copy });

		VulkanHelpers::EndSingleTimeCommands(cmd);
	}

	void* VulkanBuffer::Map(vk::DeviceSize size) const
	{
		const vk::Device device = VulkanRenderer::GetDevice();
		return device.mapMemory(m_Memory, 0, size);
	}

	void VulkanBuffer::Unmap() const
	{
		const vk::Device device = VulkanRenderer::GetDevice();
		device.unmapMemory(m_Memory);
	}
}
