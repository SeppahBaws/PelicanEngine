#include "PelicanPCH.h"
#include "AssetManager.h"

#include <logtools.h>


#include "BaseAsset.h"
#include "Pelican/Renderer/VulkanTexture.h"

namespace Pelican
{
	VulkanTexture* AssetManager::LoadTexture(const std::string& filePath, VulkanTexture::TextureMode textureMode)
	{
		const bool bAssetExists = m_TextureMap.find(filePath) != m_TextureMap.end();

		// If it already exists, simply up the refCount and return the pointer
		if (bAssetExists)
		{
			m_TextureMap[filePath].refCount += 1;
			return m_TextureMap[filePath].pAsset;
		}

		// Else, we need to load the asset and register it in our asset map
		VulkanTexture* pTexture = new VulkanTexture(filePath, textureMode); // Loads the texture into GPU memory.
		AssetReference<VulkanTexture> ref = {
			pTexture,
			1
		};
		m_TextureMap[filePath] = ref;
		m_TextureMap.insert(std::make_pair(filePath, ref));
		return pTexture;
	}

	void AssetManager::UnloadTexture(VulkanTexture* pAsset)
	{
		const std::string assetPath = pAsset->GetAssetPath();

		// Don't unload the asset when other items are pointing to it.
		if (m_TextureMap[assetPath].refCount > 1)
		{
			m_TextureMap[assetPath].refCount -= 1;
			return;
		}

		// If this is the last item pointing to this asset, we can safely unload it.
		m_TextureMap.extract(assetPath); // remove the element from the map.
		delete pAsset; // Releases the texture from GPU memory.
	}
}
