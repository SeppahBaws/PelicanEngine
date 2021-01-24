#include "PelicanPCH.h"
#include "Entity.h"

namespace Pelican
{
	Entity::Entity(const entt::entity& e)
		: m_Entity(e), m_pScene(nullptr)
	{
	}
}
