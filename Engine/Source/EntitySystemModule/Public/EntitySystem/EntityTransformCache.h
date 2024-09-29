#pragma once

#include "EntitySystem/EntityID.h"

#include <CoreUtilities/CompilerTraits.h>
#include <CoreUtilities/Containers/Map.h>
#include <CoreUtilities/Math/TQS.h>

#include <shared_mutex>

namespace Volt
{
	class VTES_API EntityTransformCache
	{
	public:
		EntityTransformCache() = default;
		~EntityTransformCache() = default;

		void CacheTransform(EntityID entityId, const TQS& transform);
		void InvalidateTransform(EntityID entityId);
		const TQS& GetCachedTransform(EntityID entityId) const;

		VT_NODISCARD VT_INLINE bool HasCachedTransform(EntityID entityId) const { ReadLock lock{ m_mutex }; return m_transformCache.contains(entityId); }

	private:
		using WriteLock = std::unique_lock<std::shared_mutex>;
		using ReadLock = std::shared_lock<std::shared_mutex>;

		vt::map<EntityID, TQS> m_transformCache;
		mutable std::shared_mutex m_mutex;
	};
}
