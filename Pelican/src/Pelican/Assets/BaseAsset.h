#pragma once

namespace Pelican
{
	class BaseAsset
	{
	public:
		BaseAsset(const std::filesystem::path& path);

		[[nodiscard]] const std::filesystem::path& GetAssetPath() const { return m_AssetPath; }

	protected:
		std::filesystem::path m_AssetPath;
	};
}
