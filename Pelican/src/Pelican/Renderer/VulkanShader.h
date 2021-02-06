#pragma once
#include <vulkan/vulkan.h>

namespace Pelican
{
	class VulkanShader
	{
	public:
		VulkanShader(std::string vertPath, std::string fragPath);
		~VulkanShader();

		std::vector<VkPipelineShaderStageCreateInfo> GetShaderStages() const { return m_ShaderStages; }

		void Reload();

	private:
		void Initialize();
		void Cleanup();

		std::vector<char> ReadFile(const std::string& filename);
		VkShaderModule CreateShaderModule(const std::vector<char>& code);

	private:
		std::string m_VertPath;
		std::string m_FragPath;

		VkShaderModule m_VertModule;
		VkShaderModule m_FragModule;
		std::vector<VkPipelineShaderStageCreateInfo> m_ShaderStages;
	};
}
