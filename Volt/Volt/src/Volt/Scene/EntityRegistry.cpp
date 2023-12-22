#include "vtpch.h"
#include "EntityRegistry.h"

#include "Volt/Scene/Entity.h"

namespace Volt
{
	void EntityRegistry::MarkEntityAsEdited(const Entity& entity)
	{
		m_editedEntities.emplace(entity.GetID());
	}

	void EntityRegistry::ClearEditedEntities()
	{
		m_editedEntities.clear();
		m_removedEntities.clear();
	}

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

		if (m_editedEntities.contains(entity.GetID()))
		{
			m_editedEntities.erase(entity.GetID());
		}

		m_removedEntities.emplace(entity.GetID());
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
