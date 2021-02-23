#include "PelicanPCH.h"
#include "SceneSerializer.h"

#include <json.hpp>
#include <glm/vec3.hpp>

namespace nlohmann
{
	// template<typename T>
	// struct adl_serializer<glm::vec3>
	// {
	// 	static void to_json(json& j, const glm::vec3& vec)
	// 	{
	// 		j = json::array({ vec.x, vec.y, vec.z });
	// 	}
	//
	// 	static void from_json(const json& j, glm::vec3& vec)
	// 	{
	// 		if (j.is_null())
	// 			vec = glm::vec3();
	// 		else
	// 		{
	// 			vec = j.get<T>();
	// 		}
	// 	}
	// };
}
