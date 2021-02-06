#include "PelicanPCH.h"
#include "VulkanShader.h"

#include <fstream>

#include "VulkanHelpers.h"
#include "VulkanRenderer.h"

namespace Pelican
{
	VulkanShader::VulkanShader(std::string vertPath, std::string fragPath)
		: m_VertPath(std::move(vertPath))
		, m_FragPath(std::move(fragPath))
	{
		Initialize();
	}

	VulkanShader::~VulkanShader()
	{
		Cleanup();
	}

	void VulkanShader::Reload()
	{
		Cleanup();
		Initialize();
	}

	void VulkanShader::Initialize()
	{
		const std::vector<char> vertCode = ReadFile(m_VertPath);
		const std::vector<char> fragCode = ReadFile(m_FragPath);

		m_VertModule = CreateShaderModule(vertCode);
		m_FragModule = CreateShaderModule(fragCode);

		VkPipelineShaderStageCreateInfo vertStageInfo = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
		vertStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertStageInfo.module = m_VertModule;
		vertStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo fragStageInfo = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
		fragStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragStageInfo.module = m_FragModule;
		fragStageInfo.pName = "main";

		m_ShaderStages = { vertStageInfo, fragStageInfo };
	}

	void VulkanShader::Cleanup()
	{
		vkDestroyShaderModule(VulkanRenderer::GetDevice(), m_VertModule, nullptr);
		vkDestroyShaderModule(VulkanRenderer::GetDevice(), m_FragModule, nullptr);
	}

	std::vector<char> VulkanShader::ReadFile(const std::string& filename)
	{
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		ASSERT_MSG(file.is_open(), "Failed to open file!");

		size_t fileSize = static_cast<size_t>(file.tellg());
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);
		file.close();

		return buffer;
	}

	VkShaderModule VulkanShader::CreateShaderModule(const std::vector<char>& code)
	{
		VkShaderModuleCreateInfo createInfo = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		VkShaderModule shaderModule;
		VK_CHECK(vkCreateShaderModule(VulkanRenderer::GetDevice(), &createInfo, nullptr, &shaderModule));

		return shaderModule;
	}
}
