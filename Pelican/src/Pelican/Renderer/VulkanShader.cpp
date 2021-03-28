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

		vk::PipelineShaderStageCreateInfo vertStageInfo{};
		vertStageInfo.setStage(vk::ShaderStageFlagBits::eVertex);
		vertStageInfo.setModule(m_VertModule);
		vertStageInfo.setPName("main");

		vk::PipelineShaderStageCreateInfo fragStageInfo;
		fragStageInfo.setStage(vk::ShaderStageFlagBits::eFragment);
		fragStageInfo.setModule(m_FragModule);
		fragStageInfo.setPName("main");

		m_ShaderStages = { vertStageInfo, fragStageInfo };
	}

	void VulkanShader::Cleanup()
	{
		VulkanRenderer::GetDevice().destroyShaderModule(m_VertModule);
		VulkanRenderer::GetDevice().destroyShaderModule(m_FragModule);
	}

	std::vector<char> VulkanShader::ReadFile(const std::string& filename) const
	{
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open())
			throw std::runtime_error("Failed to open file!");

		const size_t fileSize = static_cast<size_t>(file.tellg());
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);
		file.close();

		return buffer;
	}

	vk::ShaderModule VulkanShader::CreateShaderModule(const std::vector<char>& code) const
	{
		vk::ShaderModuleCreateInfo createInfo{};
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		vk::ShaderModule shaderModule;

		try
		{
			shaderModule = VulkanRenderer::GetDevice().createShaderModule(createInfo);
		}
		catch (vk::SystemError& e)
		{
			throw std::runtime_error("Failed to create shader module: "s + e.what());
		}

		return shaderModule;
	}
}
