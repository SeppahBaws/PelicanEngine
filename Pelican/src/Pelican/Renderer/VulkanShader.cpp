#include "PelicanPCH.h"
#include "VulkanShader.h"

#include "Pelican/IO/FileUtils.h"

#include "VulkanHelpers.h"
#include "VulkanRenderer.h"
#include "VulkanDebug.h"

namespace Pelican
{
	ShaderModule::ShaderModule(ShaderType type, const std::string& filePath)
		: m_Type(type), m_FilePath(filePath)
	{
		Initialize();
	}

	ShaderModule::ShaderModule(ShaderModule&& other) noexcept
	{
		m_FilePath = other.m_FilePath;
		m_Type = other.m_Type;
		m_ShaderModule = other.m_ShaderModule;
		m_ShaderInfo = other.m_ShaderInfo;

		other.m_ShaderModule = nullptr;
	}

	ShaderModule& ShaderModule::operator=(ShaderModule&& other) noexcept
	{
		m_FilePath = other.m_FilePath;
		m_Type = other.m_Type;
		m_ShaderModule = other.m_ShaderModule;
		m_ShaderInfo = other.m_ShaderInfo;

		other.m_ShaderModule = nullptr;

		return *this;
	}

	ShaderModule::~ShaderModule()
	{
		Cleanup();
	}

	void ShaderModule::Reload()
	{
		Cleanup();
		Initialize();
	}

	void ShaderModule::Initialize()
	{
		std::string buf;
		if (!IO::ReadFileSync(m_FilePath, buf))
		{
			throw std::runtime_error("Failed to read shader file: "s + m_FilePath);
		}

		std::vector<char> contents(buf.begin(), buf.end());

		const vk::ShaderModuleCreateInfo createInfo = vk::ShaderModuleCreateInfo()
			.setCodeSize(contents.size())
			.setPCode(reinterpret_cast<const uint32_t*>(contents.data()));

		try
		{
			m_ShaderModule = VulkanRenderer::GetDevice().createShaderModule(createInfo);
			VkDebugMarker::SetShaderModuleName(VulkanRenderer::GetDevice(), m_ShaderModule, m_FilePath.c_str());
		}
		catch (vk::SystemError& e)
		{
			throw std::runtime_error("Failed to create shader module: "s + e.what());
		}

		m_ShaderInfo = vk::PipelineShaderStageCreateInfo()
			.setStage(static_cast<vk::ShaderStageFlagBits>(m_Type))
			.setModule(m_ShaderModule)
			.setPName("main");
	}

	void ShaderModule::Cleanup() const
	{
		if (m_ShaderModule)
		{
			VulkanRenderer::GetDevice().destroyShaderModule(m_ShaderModule);
		}
	}

	void VulkanShader::AddShader(ShaderType type, const std::string& path)
	{
#ifdef PELICAN_DEBUG
		if (m_ShaderModules.find(type) != m_ShaderModules.end())
		{
			throw std::runtime_error("Shader \""s + path + "\" already exists in this collection!");
		}
#endif

		m_ShaderModules[type] = ShaderModule(type, path);
	}

	void VulkanShader::Reload()
	{
		auto it = m_ShaderModules.begin();
		while (it != m_ShaderModules.end())
		{
			it->second.Reload();
			++it;
		}
	}

	vk::ShaderModule VulkanShader::GetShaderModule(ShaderType type) const
	{
#ifdef PELICAN_DEBUG
		if (m_ShaderModules.find(type) == m_ShaderModules.end())
		{
			throw std::runtime_error("Shader module of this type was not found!");
		}
#endif
		
		return m_ShaderModules.at(type).GetShaderModule();
	}

	vk::PipelineShaderStageCreateInfo VulkanShader::GetShaderStage(ShaderType type) const
	{
#ifdef PELICAN_DEBUG
		if (m_ShaderModules.find(type) == m_ShaderModules.end())
		{
			throw std::runtime_error("Shader module of this type was not found!");
		}
#endif

		return  m_ShaderModules.at(type).GetShaderInfo();
	}

	std::vector<vk::ShaderModule> VulkanShader::GetAllShaderModules() const
	{
		std::vector<vk::ShaderModule> modules;

		auto it = m_ShaderModules.begin();
		while (it != m_ShaderModules.end())
		{
			modules.push_back(it->second.GetShaderModule());
			++it;
		}

		return modules;
	}

	std::vector<vk::PipelineShaderStageCreateInfo> VulkanShader::GetAllShaderStages() const
	{
		std::vector<vk::PipelineShaderStageCreateInfo> infos;

		auto it = m_ShaderModules.begin();
		while (it != m_ShaderModules.end())
		{
			infos.push_back(it->second.GetShaderInfo());
			++it;
		}

		return infos;
	}
}
