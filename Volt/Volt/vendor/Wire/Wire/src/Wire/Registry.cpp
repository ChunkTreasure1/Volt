#include "Registry.h"

#include "Serialization.h"
#include <random>

namespace Wire
{
	static std::random_device s_randomDevice;
	static std::mt19937 s_engine(s_randomDevice());
	static std::uniform_int_distribution<uint32_t> s_uniformDistribution;

	Registry::Registry(const Registry& registry)
	{
		m_nextEntityId = registry.m_nextEntityId;
		m_pools = registry.m_pools;
	}

	Registry::~Registry()
	{
		Clear();
	}

	EntityId Registry::CreateEntity()
	{
		EntityId id = s_uniformDistribution(s_engine);

		while (std::find(m_usedIds.begin(), m_usedIds.end(), id) != m_usedIds.end())
		{
			id = s_uniformDistribution(s_engine);
		}

		m_usedIds.emplace_back(id);
		return id;
	}

	EntityId Registry::AddEntity(EntityId aId)
	{
		assert(aId != 0);

		if (std::find(m_usedIds.begin(), m_usedIds.end(), aId) != m_usedIds.end())
		{
			while (std::find(m_usedIds.begin(), m_usedIds.end(), aId) != m_usedIds.end())
			{
				aId = s_uniformDistribution(s_engine);
			}
		}

 		if (m_nextEntityId <= aId)
		{
			m_nextEntityId = aId + 1;
		}

		m_usedIds.emplace_back(aId);

		return aId;
	}

	void Registry::RemoveEntity(EntityId aId)
	{
		assert(aId != 0);
		assert(std::find(m_usedIds.begin(), m_usedIds.end(), aId) != m_usedIds.end());

		for (auto& [guid, pool] : m_pools)
		{
			if (pool->HasComponent(aId))
			{
				pool->RemoveComponent(aId);
			}
		}

		auto it = std::find(m_usedIds.begin(), m_usedIds.end(), aId);
		m_usedIds.erase(it);
	}

	void Registry::Clear()
	{
		m_pools.clear();
		m_usedIds.clear();
		m_nextEntityId = 1;
	}

	void Registry::Sort(std::function<bool(const uint32_t lhs, const uint32_t rhs)> sortFunc)
	{
		std::sort(m_usedIds.begin(), m_usedIds.end(), sortFunc);
	}

	bool Registry::Exists(EntityId aId) const
	{
		return std::find(m_usedIds.begin(), m_usedIds.end(), aId) != m_usedIds.end();
	}

	bool Registry::HasComponent(WireGUID guid, EntityId id) const
	{
		if (m_pools.find(guid) == m_pools.end())
		{
			return false;
		}

		return m_pools.at(guid)->HasComponent(id);
	}

	void Registry::RemoveComponent(WireGUID guid, EntityId id)
	{
		if (m_pools.find(guid) == m_pools.end())
		{
			return;
		}

		m_pools.at(guid)->RemoveComponent(id);
	}

	void* Registry::GetComponentPtr(WireGUID guid, EntityId id) const
	{
		assert(HasComponent(guid, id));
		return m_pools.at(guid)->GetComponent(id);
	}

	void* Registry::AddComponent(WireGUID guid, EntityId id)
	{
		assert(!HasComponent(guid, id));

		if (m_pools.find(guid) == m_pools.end())
		{
			const auto info = ComponentRegistry::GetRegistryDataFromGUID(guid);
			if (info.size == 0)
			{
				return nullptr;
			}

			m_pools[guid] = info.poolCreateMethod();
			return m_pools[guid]->AddComponent(id, nullptr);
		}
		else
		{
			return m_pools.at(guid)->AddComponent(id, nullptr);
		}
	}

}
