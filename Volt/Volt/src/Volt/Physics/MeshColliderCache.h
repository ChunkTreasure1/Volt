#pragma once

#include "Volt/Core/Buffer.h"
#include "Volt/Physics/CookingFactory.h"

#include <unordered_map>

namespace Volt
{
	class MeshColliderCache
	{
	public:
		static void Initialize();
		static void Shutdown();

		static const bool IsCached(const std::string& colliderName);
		static void AddToCache(const std::string& colliderName, const MeshColliderCacheData& cacheData);
		static void LoadCached(const std::string& colliderName);
		static void SaveToFile(const std::string& colliderName, const MeshColliderCacheData& cacheData);

		static MeshColliderCacheData Get(const std::string& colliderName);

	private:
		MeshColliderCache() = delete;
		inline static std::unordered_map<std::string, MeshColliderCacheData> myCache;
	};
}