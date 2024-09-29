#include "espch.h"

#include "EntitySystem/EntityTransformCache.h"

namespace Volt
{
	void EntityTransformCache::CacheTransform(EntityID entityId, const TQS& transform)
	{
		WriteLock lock{ m_mutex };
		m_transformCache[entityId] = transform;
	}

	void EntityTransformCache::InvalidateTransform(EntityID entityId)
	{
		WriteLock lock{ m_mutex };
		if (m_transformCache.contains(entityId))
		{
			m_transformCache.erase(entityId);
		}
	}

	const TQS& EntityTransformCache::GetCachedTransform(EntityID entityId) const
	{
		ReadLock lock{ m_mutex };

		VT_ENSURE(m_transformCache.contains(entityId));
		return m_transformCache.at(entityId);
	}
}
