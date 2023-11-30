#pragma once

#include "Volt/Scene/EntityID.h"

#include <entt.hpp>

namespace Volt
{
	class Entity;

	class EntityRegistry
	{
	public:
		void AddEntity(const Entity& entity);
		void RemoveEntity(const Entity& entity);

		EntityID GetUUIDFromHandle(entt::entity handle) const;
		entt::entity GetHandleFromUUID(EntityID uuid) const;

		const bool Contains(EntityID uuid) const;
		const bool Contains(entt::entity handle) const;

	private:
		std::unordered_map<EntityID, entt::entity> m_entityMap;
		std::unordered_map<entt::entity, EntityID> m_handleMap;
	};
}
