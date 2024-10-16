#include "vtpch.h"
#include "Volt/Physics/MeshColliderCache.h"

#include <Volt-Core/Project/ProjectManager.h>

#include <CoreUtilities/FileIO/BinaryStreamWriter.h>
#include <CoreUtilities/FileSystem.h>

namespace Volt
{
	namespace Utility
	{
		inline static std::filesystem::path GetOrCreateCachePath()
		{
			auto path = ProjectManager::GetCachePath() / "Colliders";
			if (!FileSystem::Exists(path))
			{
				FileSystem::CreateDirectories(path);
			}
			return path;
		}
	}

	void MeshColliderCache::Initialize()
	{
	}

	void MeshColliderCache::Shutdown()
	{
		for (auto& [name, collCache] : myCache)
		{
			for (auto& data : collCache.colliderData)
			{
				data.data.Release();
			}
		}
	}

	const bool MeshColliderCache::IsCached(const std::string& colliderName)
	{
		return myCache.contains(colliderName) || FileSystem::Exists(Utility::GetOrCreateCachePath() / (colliderName + ".vtmeshcoll"));
	}

	void MeshColliderCache::AddToCache(const std::string& colliderName, const MeshColliderCacheData& cacheData)
	{
		if (myCache.contains(colliderName))
		{
			VT_LOG(Warning, "Trying to add collider with name {0}, but it already exists in the cache!", colliderName);
			return;
		}

		myCache.emplace(colliderName, cacheData);
	}

	void MeshColliderCache::LoadCached(const std::string& colliderName)
	{
		const std::filesystem::path cachedPath = Utility::GetOrCreateCachePath() / (colliderName + ".vtmeshcoll");

		if (!FileSystem::Exists(cachedPath))
		{
			VT_LOG(Warning, "Unable to load cached mesh {0}!", cachedPath.string());
			return;
		}

		std::ifstream input{ cachedPath, std::ios::binary | std::ios::in };
		if (!input.is_open())
		{
			VT_LOG(Warning, "Unable to load cached mesh {0}!", cachedPath.string());
			return;
		}

		Vector<uint8_t> totalData;
		const size_t srcSize = input.seekg(0, std::ios::end).tellg();
		totalData.resize(srcSize);
		input.seekg(0, std::ios::beg);
		input.read(reinterpret_cast<char*>(totalData.data()), totalData.size());
		input.close();

		MeshColliderCacheData cachedData;

		size_t offset = 0;
		const size_t count = *(size_t*)&totalData[offset];
		offset += sizeof(size_t);

		for (size_t i = 0; i < count; i++)
		{
			const glm::mat4 transform = *(glm::mat4*)&totalData[offset];
			offset += sizeof(transform);

			const size_t collSize = *(size_t*)&totalData[offset];
			offset += sizeof(offset);

			Vector<uint8_t> collData;
			collData.resize(collSize);

			auto& cached = cachedData.colliderData.emplace_back();
			cached.data.Allocate(collSize);
			cached.data.Copy(&totalData[offset], collSize);
			cached.transform = transform;

			offset += collSize;
		}

		myCache.emplace(colliderName, cachedData);
	}

	void MeshColliderCache::SaveToFile(const std::string& colliderName, const MeshColliderCacheData& cacheData)
	{
		const std::filesystem::path cachedPath = Utility::GetOrCreateCachePath() / (colliderName + ".vtmeshcoll");
		BinaryStreamWriter streamWriter;

		const size_t count = cacheData.colliderData.size();
		streamWriter.Write(count);

		for (const auto& mesh : cacheData.colliderData)
		{
			const size_t size = mesh.data.GetSize();

			streamWriter.Write(mesh.transform);
			streamWriter.Write(size);
			streamWriter.Write(mesh.data);
		}

		streamWriter.WriteToDisk(cachedPath, false, 0);
		myCache.emplace(colliderName, cacheData);
	}

	void MeshColliderCache::AddTriangleDebugMesh(AssetHandle meshHandle, Ref<Mesh> mesh)
	{
		myTriangleDebugMeshes[meshHandle] = mesh;
	}

	void MeshColliderCache::AddConvexDebugMesh(AssetHandle meshHandle, Ref<Mesh> mesh)
	{
		myConvexDebugMeshes[meshHandle] = mesh;
	}

	Ref<Mesh> MeshColliderCache::GetTriangleDebugMesh(AssetHandle meshHandle)
	{
		if (!myTriangleDebugMeshes.contains(meshHandle))
		{
			return nullptr;
		}

		return myTriangleDebugMeshes.at(meshHandle);
	}

	Ref<Mesh> MeshColliderCache::GetConvexDebugMesh(AssetHandle meshHandle)
	{
		if (!myConvexDebugMeshes.contains(meshHandle))
		{
			return nullptr;
		}

		return myConvexDebugMeshes.at(meshHandle);
	}

	MeshColliderCacheData MeshColliderCache::Get(const std::string& colliderName)
	{
		if (IsCached(colliderName))
		{
			if (myCache.find(colliderName) != myCache.end())
			{
				return myCache.at(colliderName);
			}
			else
			{
				LoadCached(colliderName);
				return myCache.at(colliderName);
			}
		}

		return {};
	}
}
