#include "PelicanPCH.h"
#include "VulkanDebug.h"

#include <logtools.h>

namespace Pelican
{
	namespace VkDebug
	{
		PFN_vkCreateDebugUtilsMessengerEXT g_VkCreateDebugUtilsMessenger;
		PFN_vkDestroyDebugUtilsMessengerEXT g_VkDestroyDebugUtilsMessenger;
		VkDebugUtilsMessengerEXT debugMessenger;

		VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void*)
		{
			const char* msgType = "";

			switch (messageType)
			{
			case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
				msgType = "GENERAL    ";
				break;
			case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
				msgType = "VALIDATION ";
				break;
			case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
				msgType = "PERFORMANCE";
				break;
			default:
				break;
			}

			const char* fmt = "[VULKAN] (%s) %s";

			switch (severity)
			{
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
				Logger::LogTrace(fmt, msgType, pCallbackData->pMessage);
				break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
				Logger::LogInfo(fmt, msgType, pCallbackData->pMessage);
				break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
				Logger::LogWarning(fmt, msgType, pCallbackData->pMessage);
				break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
				Logger::LogError(fmt, msgType, pCallbackData->pMessage);
				break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT:
				Logger::LogDebug(fmt, msgType, pCallbackData->pMessage);
				break;
			default: break;
			}

			return VK_FALSE;
		}

		void Setup(VkInstance instance)
		{
			g_VkCreateDebugUtilsMessenger = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
			g_VkDestroyDebugUtilsMessenger = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));

			VkDebugUtilsMessengerCreateInfoEXT info = { VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
			info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
			info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
				| VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
			info.pfnUserCallback = DebugCallback;

			ASSERT(g_VkCreateDebugUtilsMessenger(instance, &info, nullptr, &debugMessenger) == VK_SUCCESS);
		}

		void FreeDebugCallback(VkInstance instance)
		{
			if (debugMessenger != VK_NULL_HANDLE)
			{
				g_VkDestroyDebugUtilsMessenger(instance, debugMessenger, nullptr);
			}
		}
	}

	namespace VkDebugMarker
	{
		bool active = false;

		PFN_vkDebugMarkerSetObjectTagEXT g_VkDebugMarkerSetObjectTag;
		PFN_vkDebugMarkerSetObjectNameEXT g_VkDebugMarkerSetObjectName;
		PFN_vkCmdDebugMarkerBeginEXT g_VkCmdDebugMarkerBegin;
		PFN_vkCmdDebugMarkerEndEXT g_VkCmdDebugMarkerEnd;
		PFN_vkCmdDebugMarkerInsertEXT g_VkCmdDebugMarkerInsert;

		void Setup(VkDevice device)
		{
			g_VkDebugMarkerSetObjectTag = reinterpret_cast<PFN_vkDebugMarkerSetObjectTagEXT>(vkGetDeviceProcAddr(device, "vkDebugMarkerSetObjectTagEXT"));
			g_VkDebugMarkerSetObjectName = reinterpret_cast<PFN_vkDebugMarkerSetObjectNameEXT>(vkGetDeviceProcAddr(device, "vkDebugMarkerSetObjectNameEXT"));
			g_VkCmdDebugMarkerBegin = reinterpret_cast<PFN_vkCmdDebugMarkerBeginEXT>(vkGetDeviceProcAddr(device, "vkCmdDebugMarkerBeginEXT"));
			g_VkCmdDebugMarkerEnd = reinterpret_cast<PFN_vkCmdDebugMarkerEndEXT>(vkGetDeviceProcAddr(device, "vkCmdDebugMarkerEndEXT"));
			g_VkCmdDebugMarkerInsert = reinterpret_cast<PFN_vkCmdDebugMarkerInsertEXT>(vkGetDeviceProcAddr(device, "vkCmdDebugMarkerInsertEXT"));

			active = g_VkDebugMarkerSetObjectName != VK_NULL_HANDLE;
		}

		void SetObjectName(VkDevice device, uint64_t object, VkDebugReportObjectTypeEXT objectType, const char* name)
		{
			if (g_VkDebugMarkerSetObjectName)
			{
				VkDebugMarkerObjectNameInfoEXT info = { VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT };
				info.objectType = objectType;
				info.object = object;
				info.pObjectName = name;
				g_VkDebugMarkerSetObjectName(device, &info);
			}
		}

		void SetObjectTag(VkDevice device, uint64_t object, VkDebugReportObjectTypeEXT objectType, uint64_t name, size_t tagSize, const void* tag)
		{
			if (g_VkDebugMarkerSetObjectTag)
			{
				VkDebugMarkerObjectTagInfoEXT info = { VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_TAG_INFO_EXT };
				info.objectType = objectType;
				info.object = object;
				info.tagName = name;
				info.tagSize = tagSize;
				info.pTag = tag;
				g_VkDebugMarkerSetObjectTag(device, &info);
			}
		}

		void BeginRegion(VkCommandBuffer commandBuffer, const char* markerName, const glm::vec4& color)
		{
			if (g_VkCmdDebugMarkerBegin)
			{
				VkDebugMarkerMarkerInfoEXT info = { VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT };
				memcpy(info.color, &color[0], sizeof(float) * 4);
				info.pMarkerName = markerName;
				g_VkCmdDebugMarkerBegin(commandBuffer, &info);
			}
		}

		void Insert(VkCommandBuffer commandBuffer, const char* markerName, const glm::vec4& color)
		{
			if (g_VkCmdDebugMarkerInsert)
			{
				VkDebugMarkerMarkerInfoEXT info = { VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT };
				memcpy(info.color, &color[0], sizeof(float) * 4);
				info.pMarkerName = markerName;
				g_VkCmdDebugMarkerInsert(commandBuffer, &info);
			}
		}

		void EndRegion(VkCommandBuffer commandBuffer)
		{
			if (g_VkCmdDebugMarkerEnd)
			{
				g_VkCmdDebugMarkerEnd(commandBuffer);
			}
		}


		void SetCommandBufferName(VkDevice device, VkCommandBuffer commandBuffer, const char* name)
		{
			SetObjectName(device, reinterpret_cast<uint64_t>(commandBuffer), VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, name);
		}

		void SetQueueName(VkDevice device, VkQueue queue, const char* name)
		{
			SetObjectName(device, reinterpret_cast<uint64_t>(queue), VK_DEBUG_REPORT_OBJECT_TYPE_QUEUE_EXT, name);
		}

		void SetImageName(VkDevice device, VkImage image, const char* name)
		{
			SetObjectName(device, reinterpret_cast<uint64_t>(image), VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, name);
		}

		void SetSamplerName(VkDevice device, VkSampler sampler, const char* name)
		{
			SetObjectName(device, reinterpret_cast<uint64_t>(sampler), VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_EXT, name);
		}

		void SetBufferName(VkDevice device, VkBuffer buffer, const char* name)
		{
			SetObjectName(device, reinterpret_cast<uint64_t>(buffer), VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, name);
		}

		void SetDeviceMemoryName(VkDevice device, VkDeviceMemory memory, const char* name)
		{
			SetObjectName(device, reinterpret_cast<uint64_t>(memory), VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT, name);
		}

		void SetShaderModuleName(VkDevice device, VkShaderModule shaderModule, const char* name)
		{
			SetObjectName(device, reinterpret_cast<uint64_t>(shaderModule), VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT, name);
		}

		void SetPipelineName(VkDevice device, VkPipeline pipeline, const char* name)
		{
			SetObjectName(device, reinterpret_cast<uint64_t>(pipeline), VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, name);
		}

		void SetPipelineLayoutName(VkDevice device, VkPipelineLayout pipelineLayout, const char* name)
		{
			SetObjectName(device, reinterpret_cast<uint64_t>(pipelineLayout), VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT, name);
		}

		void SetRenderPassName(VkDevice device, VkRenderPass renderPass, const char* name)
		{
			SetObjectName(device, reinterpret_cast<uint64_t>(renderPass), VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT, name);
		}

		void SetFramebufferName(VkDevice device, VkFramebuffer framebuffer, const char* name)
		{
			SetObjectName(device, reinterpret_cast<uint64_t>(framebuffer), VK_DEBUG_REPORT_OBJECT_TYPE_FRAMEBUFFER_EXT, name);
		}

		void SetDescriptorSetLayoutName(VkDevice device, VkDescriptorSetLayout descriptorSetLayout, const char* name)
		{
			SetObjectName(device, reinterpret_cast<uint64_t>(descriptorSetLayout), VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT, name);
		}

		void SetDescriptorSetName(VkDevice device, VkDescriptorSet descriptorSet, const char* name)
		{
			SetObjectName(device, reinterpret_cast<uint64_t>(descriptorSet), VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT, name);
		}

		void SetSemaphoreName(VkDevice device, VkSemaphore semaphore, const char* name)
		{
			SetObjectName(device, reinterpret_cast<uint64_t>(semaphore), VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT, name);
		}

		void SetFenceName(VkDevice device, VkFence fence, const char* name)
		{
			SetObjectName(device, reinterpret_cast<uint64_t>(fence), VK_DEBUG_REPORT_OBJECT_TYPE_FENCE_EXT, name);
		}

		void SetEventName(VkDevice device, VkEvent _event, const char* name)
		{
			SetObjectName(device, reinterpret_cast<uint64_t>(_event), VK_DEBUG_REPORT_OBJECT_TYPE_EVENT_EXT, name);
		}
	}
}
