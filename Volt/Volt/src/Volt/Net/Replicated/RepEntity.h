#pragma once
#include <Nexus/Interface/Replication/Replicated.h>
#include "Volt/Scene/Entity.h"

namespace Volt
{
	class RepEntity : public Nexus::Replicated
	{
	public:
		RepEntity(Wire::EntityId in_entityId, Nexus::TYPE::CLIENT_ID in_ownerId, Volt::AssetHandle in_handle, bool preplaced = false)
			: Replicated(Nexus::TYPE::eReplicatedType::ENTITY, in_ownerId), m_entityId(in_entityId), m_prefabHandle(in_handle), m_preplaced(preplaced)
		{
		}
		Wire::EntityId GetEntityId() { return m_entityId; }
		Volt::AssetHandle GetHandle() { return m_prefabHandle; }
		const bool GetPreplaced() { return m_preplaced; }

	private:
		const Volt::AssetHandle m_prefabHandle;
		const Wire::EntityId m_entityId;
		const bool m_preplaced = false;
	};
}
