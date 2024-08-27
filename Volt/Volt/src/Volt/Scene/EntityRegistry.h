#pragma once

#include <EntitySystem/EntityID.h>

#include <CoreUtilities/Containers/Map.h>

#include <entt.hpp>

namespace Volt
{
	class Entity;

	class EntityRegistry
	{
	public:
		void MarkEntityAsEdited(const Entity& entity);
		void ClearEditedEntities();

		void AddEntity(const Entity& entity);
		void RemoveEntity(const Entity& entity);

		EntityID GetUUIDFromHandle(entt::entity handle) const;
		entt::entity GetHandleFromUUID(EntityID uuid) const;

		const bool Contains(EntityID uuid) const;
		const bool Contains(entt::entity handle) const;

		inline const std::set<EntityID>& GetEditedEntities() const { return m_editedEntities; }
		inline const std::set<EntityID>& GetRemovedEntities() const { return m_removedEntities; }

	private:
		vt::map<EntityID, entt::entity> m_entityMap;
		vt::map<entt::entity, EntityID> m_handleMap;
	
		std::set<EntityID> m_editedEntities;
		std::set<EntityID> m_removedEntities;
	};
}
