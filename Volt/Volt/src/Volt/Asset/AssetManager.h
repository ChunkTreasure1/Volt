#pragma once

#include "Volt/Asset/Asset.h"

#include "Volt/Core/Base.h"
#include "Volt/Core/Profiling.h"
#include "Volt/Log/Log.h"

#include "Volt/Rendering/Texture/Texture2D.h"
#include "Volt/Asset/Mesh/Mesh.h"
#include "Volt/Project/ProjectManager.h"

#include "Volt/Utility/StringUtility.h"

#include <map>
#include <filesystem>
#include <unordered_map>
#include <shared_mutex>
#include <functional>

namespace Volt
{
	class AssetImporter;
	class AssetManager
	{
	public:
		using WriteLock = std::unique_lock<std::shared_mutex>;
		using ReadLock = std::shared_lock<std::shared_mutex>;

		AssetManager();
		~AssetManager();

		void Initialize();
		void Shutdown();

		void SaveAssetMetaFile(std::filesystem::path assetPath);
		bool HasAssetMetaFile(const std::filesystem::path& assetPath);
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
		void RemoveMetaFile(const std::filesystem::path& path);

		void RemoveFromRegistry(AssetHandle asset);
		void RemoveFromRegistry(const std::filesystem::path& path);
		void RemoveFolderFromRegistry(const std::filesystem::path& path);
		const AssetHandle AddToRegistry(const std::filesystem::path& path, AssetHandle handle = 0);

		void ReloadAsset(AssetHandle handle);
		void ReloadAsset(const std::filesystem::path& path);

		bool IsSourceFile(AssetHandle handle) const;
		static bool IsLoaded(AssetHandle handle);

		static bool IsEngineAsset(const std::filesystem::path& path);
		bool ExistsInRegistry(AssetHandle handle) const;
		bool ExistsInRegistry(const std::filesystem::path& path) const;

		inline const std::unordered_map<std::filesystem::path, AssetHandle>& GetAssetRegistry() const { return myAssetRegistry; }

		Ref<Asset> GetAssetRaw(AssetHandle assetHandle);
		Ref<Asset> QueueAssetRaw(AssetHandle assetHandle);

		static const std::filesystem::path GetFilesystemPath(AssetHandle handle);
		static const std::filesystem::path GetRelativePath(const std::filesystem::path& path);
		static const std::filesystem::path GetPathFromAssetHandle(AssetHandle handle);
		static const std::filesystem::path GetContextPath(const std::filesystem::path& path);
		static AssetType GetAssetTypeFromHandle(const AssetHandle& handle);
		static AssetType GetAssetTypeFromPath(const std::filesystem::path& path);
		static AssetType GetAssetTypeFromExtension(const std::string& extension);
		static AssetHandle GetAssetHandleFromPath(const std::filesystem::path& path);
		static std::string GetExtensionFromAssetType(AssetType type);

		static std::vector<std::filesystem::path> GetPathFromFilename(std::string filename);

		inline static AssetManager& Get() { return *s_instance; }

		template<typename T>
		static Ref<T> GetAsset(AssetHandle assetHandle);

		template<typename T>
		static Ref<T> GetAsset(const std::filesystem::path& path);

		template<typename T>
		static Ref<T> GetAssetLocking(AssetHandle assetHandle);

		template<typename T>
		static Ref<T> GetAssetLocking(const std::filesystem::path& path);

		template<typename T>
		static Ref<T> QueueAsset(const std::filesystem::path& path);

		template<typename T>
		static Ref<T> QueueAsset(AssetHandle handle);

		template<typename T>
		static Ref<T> QueueAsset(AssetHandle handle, const std::function<void()>& assetLoadedCallback);

		template<typename T, typename... Args>
		static Ref<T> CreateAsset(const std::filesystem::path& targetDir, const std::string& filename, Args&&... args);

		template<typename T, typename... Args>
		static Ref<T> CreateMemoryAsset(Args&&... args);

		template<typename ImporterType, typename Type>
		static const ImporterType& GetImporterForType();

		template<typename T>
		static const std::vector<Ref<T>> GetAllCachedAssetsOfType();

		template<typename T>
		static const std::vector<std::filesystem::path> GetAllAssetsOfType();

		static const std::vector<AssetHandle> GetAllAssetsWithDependency(const std::filesystem::path& dependencyPath);

	private:
		struct LoadJob
		{
			AssetHandle handle;
			std::filesystem::path path;
		};

		inline static AssetManager* s_instance = nullptr;

		void LoadAsset(AssetHandle assetHandle, Ref<Asset>& asset);
		void LoadAssetMetaFile(std::filesystem::path metaPath);
		void LoadAssetMetaFiles();

		void QueueAssetInternal(const std::filesystem::path& path, Ref<Asset>& asset);
		void QueueAssetInternal(AssetHandle assetHandle, Ref<Asset>& asset);

		void QueueAssetInternal(const std::filesystem::path& path, Ref<Asset>& asset, const std::function<void()>& loadedCallback);
		void QueueAssetInternal(AssetHandle assetHandle, Ref<Asset>& asset, const std::function<void()>& loadedCallback);

		static const std::filesystem::path GetCleanPath(const std::filesystem::path& path);

		std::vector<std::filesystem::path> GetMetaFiles();

		std::unordered_map<AssetType, Scope<AssetImporter>> myAssetImporters;
		std::unordered_map <AssetHandle, std::vector<std::filesystem::path>> myAssetDependencies;

		std::unordered_map<std::filesystem::path, AssetHandle> myAssetRegistry;

		std::unordered_map<AssetHandle, Ref<Asset>> myAssetCache;
		std::unordered_map<AssetHandle, Ref<Asset>> myMemoryAssets;

		mutable std::shared_mutex myAssetRegistryMutex;
		mutable std::shared_mutex myAssetCacheMutex;
	};

	template<typename T>
	inline Ref<T> AssetManager::GetAsset(AssetHandle assetHandle)
	{
		if (assetHandle == Asset::Null())
		{
			return nullptr;
		}

		Ref<Asset> asset;
		Get().LoadAsset(assetHandle, asset);

		if (asset)
		{
			if (asset->GetType() != T::GetStaticType())
			{
				AssetType assetType = asset->GetType();
				VT_CORE_CRITICAL("Type Mismatch!");
			}
		}
		return std::reinterpret_pointer_cast<T>(asset);
	}

	template<typename T>
	inline Ref<T> AssetManager::GetAsset(const std::filesystem::path& path)
	{
		return GetAsset<T>(GetAssetHandleFromPath(path));
	}

	template<typename T>
	inline Ref<T> AssetManager::GetAssetLocking(AssetHandle assetHandle)
	{
		if (assetHandle == Asset::Null())
		{
			return nullptr;
		}

		Ref<Asset> asset = QueueAsset<T>(assetHandle);
		if (!asset)
		{
			return nullptr;
		}

		while (!asset->IsValid())
		{}

		if (asset->GetType() != T::GetStaticType())
		{
			VT_CORE_CRITICAL("Type Mismatch!");
		}

		return std::reinterpret_pointer_cast<T>(asset);
	}

	template<typename T>
	inline Ref<T> AssetManager::GetAssetLocking(const std::filesystem::path& path)
	{
		return GetAsset<T>(GetAssetHandleFromPath(path));
	}

	template<typename T>
	inline Ref<T> AssetManager::QueueAsset(const std::filesystem::path& path)
	{
		// If it's already loaded, return it
		{
			ReadLock lock{ Get().myAssetRegistryMutex };
			const auto handle = GetAssetHandleFromPath(path);
			if (IsLoaded(handle))
			{
				return GetAsset<T>(handle);
			}
		}

		Ref<Asset> asset = CreateRef<T>();
		asset->SetFlag(AssetFlag::Queued, true);
		Get().QueueAssetInternal(path, asset);

		if (asset->GetType() != T::GetStaticType())
		{
			VT_CORE_CRITICAL("Type Mismatch!");
		}

		return std::reinterpret_pointer_cast<T>(asset);
	}

	template<typename T>
	inline Ref<T> AssetManager::QueueAsset(AssetHandle handle)
	{
		if (handle == Asset::Null())
		{
			return nullptr;
		}

		// If it's a memory asset, return it
		if (Get().myMemoryAssets.contains(handle))
		{
			return std::reinterpret_pointer_cast<T>(Get().myMemoryAssets.at(handle));
		}

		// If it's already loaded, return it
		{
			if (IsLoaded(handle))
			{
				return GetAsset<T>(handle);
			}
		}

		Ref<Asset> asset = CreateRef<T>();
		asset->SetFlag(AssetFlag::Queued, true);
		Get().QueueAssetInternal(handle, asset);

		if (asset->GetType() != T::GetStaticType())
		{
			VT_CORE_CRITICAL("Type Mismatch!");
		}
		return std::reinterpret_pointer_cast<T>(asset);
	}

	template<typename T>
	inline Ref<T> AssetManager::QueueAsset(AssetHandle handle, const std::function<void()>& assetLoadedCallback)
	{
		if (handle == Asset::Null())
		{
			return nullptr;
		}

		// If it's a memory asset, return it
		if (Get().myMemoryAssets.contains(handle))
		{
			return std::reinterpret_pointer_cast<T>(Get().myMemoryAssets.at(handle));
		}

		// If it's already loaded, return it
		{
			if (IsLoaded(handle))
			{
				return GetAsset<T>(handle);
			}
		}

		Ref<Asset> asset = CreateRef<T>();
		asset->SetFlag(AssetFlag::Queued, true);
		Get().QueueAssetInternal(handle, asset, assetLoadedCallback);

		if (asset->GetType() != T::GetStaticType())
		{
			VT_CORE_CRITICAL("Type Mismatch!");
		}
		return std::reinterpret_pointer_cast<T>(asset);
	}

	template<typename T, typename ...Args>
	inline Ref<T> AssetManager::CreateAsset(const std::filesystem::path& targetDir, const std::string& filename, Args && ...args)
	{
		Ref<T> asset = CreateRef<T>(std::forward<Args>(args)...);
		asset->path = Utils::ReplaceCharacter((targetDir / filename).string(), '\\', '/');

		WriteLock lockCache{ Get().myAssetCacheMutex };
		WriteLock lockRegistry{ Get().myAssetRegistryMutex };

		AssetManager::Get().myAssetRegistry.emplace(GetCleanPath(asset->path), asset->handle);
		AssetManager::Get().myAssetCache.emplace(asset->handle, asset);

		Get().SaveAssetMetaFile(asset->path);

		return asset;
	}

	template<typename T, typename ...Args>
	inline Ref<T> AssetManager::CreateMemoryAsset(Args&& ...args)
	{
		Ref<T> asset = CreateRef<T>(std::forward<Args>(args)...);
		AssetManager::Get().myMemoryAssets.emplace(asset->handle, asset);
		return asset;
	}

	template<typename ImporterType, typename Type>
	inline const ImporterType& AssetManager::GetImporterForType()
	{
		const auto type = Type::GetStaticType();
		if (!Get().myAssetImporters.contains(type))
		{
			VT_CORE_ASSERT(false, "Importer for type does not exist!");
		}

		return (ImporterType&)*Get().myAssetImporters.at(type);
	}
	template<typename T>
	inline const std::vector<Ref<T>> AssetManager::GetAllCachedAssetsOfType()
	{
		ReadLock lock{ Get().myAssetCacheMutex };

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
		ReadLock lock{ Get().myAssetCacheMutex };

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
