#pragma once

namespace Pelican
{
	class Scene;

	class SceneSerializer final
	{
	public:
		static void Serialize(const Scene* pScene, std::string& serialized);
		static void Deserialize(const std::string& serialized, Scene* pScene);
	};
}
