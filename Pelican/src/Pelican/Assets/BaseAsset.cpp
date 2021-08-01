#include "PelicanPCH.h"
#include "BaseAsset.h"

#include <functional>

namespace Pelican
{
	BaseAsset::BaseAsset(const std::filesystem::path& path)
		: m_AssetPath(path)
	{
		std::hash<std::string> hasher;
		m_Hash = hasher(path.string());
	}
}
