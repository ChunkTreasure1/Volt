#include "vtpch.h"
#include "EntityRegistry.h"

#include "Volt/Scene/Entity.h"

namespace Volt
{
	void EntityRegistry::AddEntity(const Entity& entity)
	{
		if (m_entityMap.contains(entity.GetID()) || m_handleMap.contains(entity))
		{
			return;
		}

		m_entityMap.emplace(entity.GetID(), entity);
		m_handleMap.emplace(entity, entity.GetID());
	}

	void EntityRegistry::RemoveEntity(const Entity& entity)
	{
		if (m_entityMap.contains(entity.GetID()))
		{
			m_entityMap.erase(entity.GetID());
		}

		if (m_handleMap.contains(entity))
		{
			m_handleMap.erase(entity);
		}
	}

	EntityID EntityRegistry::GetUUIDFromHandle(entt::entity handle) const
	{
		if (!m_handleMap.contains(handle))
		{
			return Entity::NullID();
		}

		return m_handleMap.at(handle);
	}

	entt::entity EntityRegistry::GetHandleFromUUID(EntityID uuid) const
	{
		if (!m_entityMap.contains(uuid))
		{
			return entt::null;
		}

		return m_entityMap.at(uuid);
	}

	const bool EntityRegistry::Contains(EntityID uuid) const
	{
		return m_entityMap.contains(uuid);
	}

	const bool EntityRegistry::Contains(entt::entity handle) const
	{
		return m_handleMap.contains(handle);
	}
}
