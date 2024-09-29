#include "espch.h"

#include "EntitySystem/EntityRegistry.h"
#include "EntitySystem/EntityHelper.h"

namespace Volt
{
	void EntityRegistry2::MarkEntityAsEdited(const EntityHelper& entity)
	{
		m_editedEntities.emplace(entity.GetID());
	}

	void EntityRegistry2::ClearEditedEntities()
	{
		m_editedEntities.clear();
		m_removedEntities.clear();
	}

	void EntityRegistry2::AddEntity(const EntityHelper& entity)
	{
		if (m_entityMap.contains(entity.GetID()) || m_handleMap.contains(entity.GetHandle()))
		{
			return;
		}

		m_entityMap.emplace(entity.GetID(), entity.GetHandle());
		m_handleMap.emplace(entity.GetHandle(), entity.GetID());
	}

	void EntityRegistry2::RemoveEntity(const EntityHelper& entity)
	{
		if (m_entityMap.contains(entity.GetID()))
		{
			m_entityMap.erase(entity.GetID());
		}

		if (m_handleMap.contains(entity.GetHandle()))
		{
			m_handleMap.erase(entity.GetHandle());
		}

		if (m_editedEntities.contains(entity.GetID()))
		{
			m_editedEntities.erase(entity.GetID());
		}

		m_removedEntities.emplace(entity.GetID());
	}

	EntityID EntityRegistry2::GetUUIDFromHandle(entt::entity handle) const
	{
		if (!m_handleMap.contains(handle))
		{
			return EntityID::Null();
		}

		return m_handleMap.at(handle);
	}

	entt::entity EntityRegistry2::GetHandleFromID(EntityID uuid) const
	{
		if (!m_entityMap.contains(uuid))
		{
			return entt::null;
		}

		return m_entityMap.at(uuid);
	}

	bool EntityRegistry2::Contains(EntityID uuid) const
	{
		return m_entityMap.contains(uuid);
	}

	bool EntityRegistry2::Contains(entt::entity handle) const
	{
		return m_handleMap.contains(handle);
	}
}
