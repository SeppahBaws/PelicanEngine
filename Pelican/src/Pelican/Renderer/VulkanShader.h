#pragma once
#include <vulkan/vulkan.hpp>

namespace Pelican
{
	enum class ShaderType : VkShaderStageFlags
	{
		Vertex = VK_SHADER_STAGE_VERTEX_BIT,
		Geometry = VK_SHADER_STAGE_GEOMETRY_BIT,
		Fragment = VK_SHADER_STAGE_FRAGMENT_BIT,
		Compute = VK_SHADER_STAGE_COMPUTE_BIT,
	};

	class ShaderModule final
	{
	public:
		ShaderModule() = default;
		ShaderModule(ShaderType type, const std::string& filePath);
		ShaderModule(const ShaderModule& other) = delete;
		ShaderModule& operator=(const ShaderModule& other) = delete;
		ShaderModule(ShaderModule&& other) noexcept;
		ShaderModule& operator=(ShaderModule&& other) noexcept;
		~ShaderModule();

		void Reload();

		vk::ShaderModule GetShaderModule() const
		{ return m_ShaderModule; }

		vk::PipelineShaderStageCreateInfo GetShaderInfo() const
		{ return m_ShaderInfo; }

	private:
		void Initialize();
		void Cleanup() const;

	private:
		ShaderType m_Type;
		std::string m_FilePath;

		vk::ShaderModule m_ShaderModule{ nullptr };
		vk::PipelineShaderStageCreateInfo m_ShaderInfo;
	};

	class VulkanShader final
	{
	public:
		void AddShader(ShaderType type, const std::string& path);

		void Reload();

		[[nodiscard]] vk::ShaderModule GetShaderModule(ShaderType type) const;
		[[nodiscard]] vk::PipelineShaderStageCreateInfo GetShaderStage(ShaderType type) const;
		[[nodiscard]] std::vector<vk::ShaderModule> GetAllShaderModules() const;
		[[nodiscard]] std::vector<vk::PipelineShaderStageCreateInfo> GetAllShaderStages() const;

	private:
		std::map<ShaderType, ShaderModule> m_ShaderModules;
	};
}
