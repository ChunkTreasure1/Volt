#pragma once

#include "EntitySystem/EntityID.h"

#include <CoreUtilities/Containers/Map.h>

#include <entt.hpp>

namespace Volt
{
	class EntityHelper;

	class EntityRegistry
	{
	public:
		void MarkEntityAsEdited(const EntityHelper& entity);
		void ClearEditedEntities();

		void AddEntity(const EntityHelper& entity);
		void RemoveEntity(const EntityHelper& entity);

		EntityID GetUUIDFromHandle(entt::entity handle) const;
		entt::entity GetHandleFromID(EntityID uuid) const;

		bool Contains(EntityID uuid) const;
		bool Contains(entt::entity handle) const;

		inline const std::set<EntityID>& GetEditedEntities() const { return m_editedEntities; }
		inline const std::set<EntityID>& GetRemovedEntities() const { return m_removedEntities; }

	private:
		vt::map<EntityID, entt::entity> m_entityMap;
		vt::map<entt::entity, EntityID> m_handleMap;

		std::set<EntityID> m_editedEntities;
		std::set<EntityID> m_removedEntities;
	};
}
