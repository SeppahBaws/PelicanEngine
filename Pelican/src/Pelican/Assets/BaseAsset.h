#pragma once

namespace Pelican
{
	class BaseAsset
	{
	public:
		BaseAsset(const std::filesystem::path& path);

		[[nodiscard]] const std::filesystem::path& GetAssetPath() const { return m_AssetPath; }
		[[nodiscard]] std::size_t GetHash() const { return m_Hash; }

		bool operator==(const BaseAsset& other) const
		{
			return other.m_Hash == m_Hash;
		}

	protected:
		std::filesystem::path m_AssetPath;
		std::size_t m_Hash;
	};
}
