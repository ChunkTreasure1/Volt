#pragma once

#include "Volt/Asset/Asset.h"

#include "Volt/Core/Base.h"
#include "Volt/Core/Profiling.h"
#include "Volt/Log/Log.h"

#include "Volt/Rendering/Texture/Texture2D.h"
#include "Volt/Asset/Mesh/Mesh.h"
#include "Volt/Project/ProjectManager.h"

#include <map>
#include <filesystem>
#include <unordered_map>

namespace Volt
{
	class AssetImporter;
	class AssetManager
	{
	public:
		AssetManager();
		~AssetManager();

		void Initialize();
		void Shutdown();

		void AddDependency(AssetHandle asset, const std::filesystem::path& dependency);
		const std::vector<std::filesystem::path>& GetDependencies(AssetHandle asset) const;

		void Unload(AssetHandle assetHandle);
		void SaveAsset(const Ref<Asset> asset);

		void MoveAsset(Ref<Asset> asset, const std::filesystem::path& targetDir);
		void MoveAsset(AssetHandle asset, const std::filesystem::path& targetDir);
		void MoveFolder(const std::filesystem::path& sourceDir, const std::filesystem::path& targetDir);
		
		void RenameAsset(AssetHandle asset, const std::string& newName);
		void RenameAssetFolder(AssetHandle asset, const std::filesystem::path& targetPath);

		void RemoveAsset(AssetHandle asset);
		void RemoveAsset(const std::filesystem::path& path);

		void RemoveFromRegistry(AssetHandle asset);
		void RemoveFromRegistry(const std::filesystem::path& path);
		void RemoveFolderFromRegistry(const std::filesystem::path& path);
		const AssetHandle AddToRegistry(const std::filesystem::path& path);

		void ReloadAsset(AssetHandle handle);
		void ReloadAsset(const std::filesystem::path& path);

		bool IsSourceFile(AssetHandle handle) const;
		bool IsLoaded(AssetHandle handle) const;
		bool ExistsInRegistry(AssetHandle handle) const;
		bool ExistsInRegistry(const std::filesystem::path& path) const;

		const std::filesystem::path GetFilesystemPath(AssetHandle handle);
		const std::filesystem::path GetRelativePath(const std::filesystem::path& path);

		inline const std::unordered_map<std::filesystem::path, AssetHandle>& GetAssetRegistry() const { return myAssetRegistry; }

		Ref<Asset> GetAssetRaw(AssetHandle assetHandle);
		Ref<Asset> QueueAssetRaw(AssetHandle assetHandle);

		static AssetType GetAssetTypeFromHandle(const AssetHandle& handle);
		static AssetType GetAssetTypeFromPath(const std::filesystem::path& path);
		static AssetType GetAssetTypeFromExtension(const std::string& extension);
		static std::filesystem::path GetPathFromAssetHandle(AssetHandle handle);
		static AssetHandle GetAssetHandleFromPath(const std::filesystem::path& path);
		static std::string GetExtensionFromAssetType(AssetType type);

		inline static AssetManager& Get() { return *s_instance; }

		template<typename T>
		static Ref<T> GetAsset(AssetHandle assetHandle);

		template<typename T>
		static Ref<T> GetAsset(const std::filesystem::path& path);

		template<typename T>
		static Ref<T> QueueAsset(const std::filesystem::path& path);

		template<typename T>
		static Ref<T> QueueAsset(AssetHandle handle);

		template<typename T, typename... Args>
		static Ref<T> CreateAsset(const std::filesystem::path& targetDir, const std::string& filename, Args&&... args);

		template<typename T>
		static const std::vector<Ref<T>> GetAllCachedAssetsOfType();

		template<typename T>
		static const std::vector<std::filesystem::path> GetAllAssetsOfType();

	private:
		struct LoadJob
		{
			AssetHandle handle;
			std::filesystem::path path;
		};

		inline static AssetManager* s_instance = nullptr;

		void LoadAsset(AssetHandle assetHandle, Ref<Asset>& asset);
		
		void QueueAssetInternal(const std::filesystem::path& path, Ref<Asset>& asset);
		void QueueAssetInternal(AssetHandle assetHandle, Ref<Asset>& asset);

		void Thread_LoadAsset();

		void SaveAssetRegistry();
		void LoadAssetRegistry();

		std::unordered_map<AssetType, Scope<AssetImporter>> myAssetImporters;
		std::unordered_map<std::filesystem::path, AssetHandle> myAssetRegistry;
		std::unordered_map<AssetHandle, Ref<Asset>> myAssetCache;
		std::unordered_map <AssetHandle, std::vector<std::filesystem::path>> myAssetDependencies;

		std::thread myLoadThread;
		std::mutex myLoadMutex;
		std::mutex myAssetRegistryMutex;

		std::atomic_bool myIsLoadThreadRunning = false;
		std::condition_variable myThreadConditionVariable;

		std::vector<LoadJob> myLoadQueue;
	};

	template<typename T>
	inline Ref<T> AssetManager::GetAsset(AssetHandle assetHandle)
	{
		VT_PROFILE_FUNCTION();

		if (assetHandle == Asset::Null())
		{
			return nullptr;
		}

		Ref<Asset> asset;
		Get().LoadAsset(assetHandle, asset);

		if (asset)
		{
			VT_CORE_ASSERT(asset->GetType() == T::GetStaticType(), "Type mismatch!")
		}
		return std::reinterpret_pointer_cast<T>(asset);
	}

	template<typename T>
	inline Ref<T> AssetManager::GetAsset(const std::filesystem::path& path)
	{
		return GetAsset<T>(GetAssetHandleFromPath(path));
	}

	template<typename T>
	inline Ref<T> AssetManager::QueueAsset(const std::filesystem::path& path)
	{
		if (!std::filesystem::exists(path))
		{
			VT_CORE_ERROR("Unable to load asset {0}! It does not exist!", path.string().c_str());
			return nullptr;
		}

		Ref<Asset> asset = CreateRef<T>();
		asset->SetFlag(AssetFlag::Queued, true);
		Get().QueueAssetInternal(path, asset);

		VT_CORE_ASSERT(asset->GetType() == T::GetStaticType(), "Type mismatch!")
		return std::reinterpret_pointer_cast<T>(asset);
	}

	template<typename T>
	inline Ref<T> AssetManager::QueueAsset(AssetHandle handle)
	{
		if (handle == Asset::Null())
		{
			return nullptr;
		}

		Ref<Asset> asset = CreateRef<T>();
		asset->SetFlag(AssetFlag::Queued, true);
		Get().QueueAssetInternal(handle, asset);

		VT_CORE_ASSERT(asset->GetType() == T::GetStaticType(), "Type mismatch!")
		return std::reinterpret_pointer_cast<T>(asset);
	}

	template<typename T, typename ...Args>
	inline Ref<T> AssetManager::CreateAsset(const std::filesystem::path& targetDir, const std::string& filename, Args && ...args)
	{
		Ref<T> asset = CreateRef<T>(std::forward<Args>(args)...);
		asset->path = targetDir / filename;

		std::scoped_lock lock(Get().myLoadMutex);

		AssetManager::Get().myAssetRegistry.emplace(asset->path, asset->handle);
		AssetManager::Get().myAssetCache.emplace(asset->handle, asset);

		Get().SaveAssetRegistry();

		return asset;
	}
	template<typename T>
	inline const std::vector<Ref<T>> AssetManager::GetAllCachedAssetsOfType()
	{
		std::vector<Ref<T>> result{};

		for (const auto& [handle, asset] : Get().myAssetCache)
		{
			if (asset->GetType() == T::GetStaticType())
			{
				result.push_back(reinterpret_pointer_cast<T>(asset));
			}
		}

		return result;
	}

	template<typename T>
	inline const std::vector<std::filesystem::path> AssetManager::GetAllAssetsOfType()
	{
		std::vector<std::filesystem::path> result{};

		for (const auto& [path, handle] : Get().myAssetRegistry)
		{
			if (GetAssetTypeFromPath(path) == T::GetStaticType())
			{
				result.emplace_back(path);
			}
		}

		return result;
	}
}