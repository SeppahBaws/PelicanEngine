#pragma once

namespace Pelican
{
	class BaseAsset
	{
	public:
		BaseAsset(const std::string& path);

		[[nodiscard]] const std::string& GetAssetPath() const { return m_AssetPath; }

	protected:
		std::string m_AssetPath;
	};
}
