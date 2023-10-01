#pragma once
#include <Nexus/Interface/Replication/Replicated.h>
#include "Volt/Scene/Entity.h"

namespace Volt
{
	class RepEntity : public Nexus::Replicated
	{
	public:
		RepEntity(entt::entity in_entityId, Nexus::TYPE::CLIENT_ID in_ownerId, Volt::AssetHandle in_handle, bool preplaced = false)
			: Replicated(Nexus::TYPE::eReplicatedType::ENTITY, in_ownerId), m_entityId(in_entityId), m_prefabHandle(in_handle), m_preplaced(preplaced)
		{
		}
		const entt::entity GetEntityId() const { return m_entityId; }
		const Volt::AssetHandle GetHandle() const { return m_prefabHandle; }
		const bool GetPreplaced() const { return m_preplaced; }

	private:
		const Volt::AssetHandle m_prefabHandle;
		const entt::entity m_entityId;
		const bool m_preplaced = false;
	};
}
