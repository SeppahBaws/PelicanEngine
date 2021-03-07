#pragma once

#include <vulkan/vulkan.h>

#pragma warning(push, 0)
#include <glm/vec4.hpp>
#pragma warning(pop)

namespace Pelican
{
	namespace VkDebug
	{
		VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void*);

		// Load debug functions and setup debug callback
		void Setup(VkInstance instance);

		void FreeDebugCallback(VkInstance instance);
	}

	namespace VkDebugMarker
	{
		// Load the required function pointers
		void Setup(VkDevice device);

		void SetObjectName(VkDevice device, uint64_t object, VkDebugReportObjectTypeEXT objectType, const char* name);

		void SetObjectTag(VkDevice device, uint64_t object, VkDebugReportObjectTypeEXT objectType, uint64_t name, size_t tagSize, const void* tag);

		void BeginRegion(VkCommandBuffer commandBuffer, const char* markerName, const glm::vec4& color);
		void Insert(VkCommandBuffer commandBuffer, const char* markerName, const glm::vec4& color);
		void EndRegion(VkCommandBuffer commandBuffer);

		void SetCommandBufferName(VkDevice device, VkCommandBuffer commandBuffer, const char* name);
		void SetQueueName(VkDevice device, VkQueue queue, const char* name);
		void SetImageName(VkDevice device, VkImage image, const char* name);
		void SetSamplerName(VkDevice device, VkSampler sampler, const char* name);
		void SetBufferName(VkDevice device, VkBuffer buffer, const char* name);
		void SetDeviceMemoryName(VkDevice device, VkDeviceMemory memory, const char* name);
		void SetShaderModuleName(VkDevice device, VkShaderModule shaderModule, const char* name);
		void SetPipelineName(VkDevice device, VkPipeline pipeline, const char* name);
		void SetPipelineLayoutName(VkDevice device, VkPipelineLayout pipelineLayout, const char* name);
		void SetRenderPassName(VkDevice device, VkRenderPass renderPass, const char* name);
		void SetFramebufferName(VkDevice device, VkFramebuffer framebuffer, const char* name);
		void SetDescriptorSetLayoutName(VkDevice device, VkDescriptorSetLayout descriptorSetLayout, const char* name);
		void SetDescriptorSetName(VkDevice device, VkDescriptorSet descriptorSet, const char* name);
		void SetSemaphoreName(VkDevice device, VkSemaphore semaphore, const char* name);
		void SetFenceName(VkDevice device, VkFence fence, const char* name);
		void SetEventName(VkDevice device, VkEvent _event, const char* name);
	}
}
