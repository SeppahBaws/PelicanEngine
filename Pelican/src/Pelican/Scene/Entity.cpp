#include "PelicanPCH.h"
#include "Entity.h"

namespace Pelican
{
	Entity::Entity(const entt::entity& e)
		: m_Entity(e), m_pScene(nullptr)
	{
	}

	Entity::Entity(const entt::entity& e, Scene* pScene)
		: m_Entity(e), m_pScene(pScene)
	{
	}
}
