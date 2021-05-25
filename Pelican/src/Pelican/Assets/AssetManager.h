#pragma once

namespace Pelican
{
	class VulkanTexture;
	class BaseAsset;

	// Simple reference-counted wrapper around a BaseAsset.
	// Basically a smart pointer, but I can't control what happens when the pointer goes invalid.
	template<class T>
	struct AssetReference
	{
		T* pAsset;
		int32_t refCount;
	};

	using TextureMap = std::unordered_map<std::string, AssetReference<VulkanTexture>>;

	class AssetManager
	{
	public:
		// Singleton - probably should use a ServiceLocator later.
		static AssetManager& GetInstance()
		{
			static AssetManager instance{};
			return instance;
		}

		VulkanTexture* LoadTexture(const std::string& file);
		void UnloadTexture(VulkanTexture* pAsset);

	private:
		TextureMap m_TextureMap;
	};
}
