#include "vtpch.h"
#include "MeshColliderCache.h"

#include "Volt/Utility/FileSystem.h"

namespace Volt
{
	void MeshColliderCache::Initialize()
	{}

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
		return myCache.find(colliderName) != myCache.end() || FileSystem::Exists(FileSystem::GetMeshColliderCache() / (colliderName + ".vtmeshcoll"));
	}

	void MeshColliderCache::AddToCache(const std::string& colliderName, const MeshColliderCacheData& cacheData)
	{
		if (myCache.find(colliderName) != myCache.end())
		{
			VT_CORE_WARN("Trying to add collider with name {0}, but it already exists in the cache!", colliderName);
			return;
		}

		myCache.emplace(colliderName, cacheData);
	}

	void MeshColliderCache::LoadCached(const std::string& colliderName)
	{
		const std::filesystem::path cachedPath = FileSystem::GetMeshColliderCache() / (colliderName + ".vtmeshcoll");
		
		if (!FileSystem::Exists(cachedPath))
		{
			VT_CORE_WARN("Unable to load cached mesh {0}!", cachedPath.string());
			return;
		}

		std::ifstream input{ cachedPath, std::ios::binary | std::ios::in };
		if (!input.is_open())
		{
			VT_CORE_WARN("Unable to load cached mesh {0}!", cachedPath.string());
			return;
		}

		std::vector<uint8_t> totalData;
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
			const size_t collSize = *(size_t*)&totalData[offset];
			offset += sizeof(offset);

			std::vector<uint8_t> collData;
			collData.resize(collSize);
		
			auto& cached = cachedData.colliderData.emplace_back();
			cached.data.Allocate(collSize);
			cached.data.Copy(&totalData[offset], collSize);

			offset += collSize;
		}

		myCache.emplace(colliderName, cachedData);
	}

	void MeshColliderCache::SaveToFile(const std::string& colliderName, const MeshColliderCacheData& cacheData)
	{
		std::vector<uint8_t> outData;
		outData.resize(sizeof(size_t));
		size_t offset = 0;

		const size_t count = cacheData.colliderData.size();
		memcpy_s(outData.data(), outData.size(), &count, sizeof(size_t));
		offset += sizeof(size_t);

		for (const auto& mesh : cacheData.colliderData)
		{
			const size_t size = mesh.data.GetSize();
			outData.resize(outData.size() + sizeof(size_t) + size);

			memcpy_s(&outData[offset], sizeof(size_t), &size, sizeof(size_t));
			offset += sizeof(size_t);

			memcpy_s(&outData[offset], size, mesh.data.As<uint8_t>(), size);
			offset += size;
		}
		
		if (!FileSystem::Exists(FileSystem::GetMeshColliderCache()))
		{
			std::filesystem::create_directories(FileSystem::GetMeshColliderCache());
		}

		const std::filesystem::path cachedPath = FileSystem::GetMeshColliderCache() / (colliderName + ".vtmeshcoll");
		std::ofstream output{ cachedPath, std::ios::binary | std::ios::out };
		if (!output.is_open())
		{
			VT_CORE_WARN("Unable to save file {0}", cachedPath.string());
			return;
		}

		output.write((char*)outData.data(), outData.size());
		output.close();

		myCache.emplace(colliderName, cacheData);
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