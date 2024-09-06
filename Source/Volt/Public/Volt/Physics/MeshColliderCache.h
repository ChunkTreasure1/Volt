#pragma once

#include <CoreUtilities/Buffer/Buffer.h>
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

		static void AddTriangleDebugMesh(AssetHandle meshHandle, Ref<Mesh> mesh);
		static void AddConvexDebugMesh(AssetHandle meshHandle, Ref<Mesh> mesh);

		static Ref<Mesh> GetTriangleDebugMesh(AssetHandle meshHandle);
		static Ref<Mesh> GetConvexDebugMesh(AssetHandle meshHandle);

		static MeshColliderCacheData Get(const std::string& colliderName);

	private:
		MeshColliderCache() = delete;
		inline static std::unordered_map<std::string, MeshColliderCacheData> myCache;
		inline static std::unordered_map<AssetHandle, Ref<Mesh>> myTriangleDebugMeshes;
		inline static std::unordered_map<AssetHandle, Ref<Mesh>> myConvexDebugMeshes;
	};
}
