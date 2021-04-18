#pragma once
#include <vulkan/vulkan.hpp>

namespace Pelican
{
	class VulkanShader
	{
	public:
		VulkanShader(std::string vertPath, std::string fragPath);
		~VulkanShader();

		const std::vector<vk::PipelineShaderStageCreateInfo>& GetShaderStages() const { return m_ShaderStages; }

		void Reload();

	private:
		void Initialize();
		void Cleanup();

		std::vector<char> ReadFile(const std::string& filename) const;
		vk::ShaderModule CreateShaderModule(const std::vector<char>& code) const;

	private:
		std::string m_VertPath;
		std::string m_FragPath;

		vk::ShaderModule m_VertModule;
		vk::ShaderModule m_FragModule;
		std::vector<vk::PipelineShaderStageCreateInfo> m_ShaderStages;
	};
}
