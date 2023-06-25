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

		void AddDependency(AssetHandle asset, const std::filesystem::path& dependency);
		void AddDependency(AssetHandle asset, AssetHandle dependency);

		bool HasAssetMetaFile(AssetHandle assetHandle);

		void Unload(AssetHandle assetHandle);

		void MoveAsset(Ref<Asset> asset, const std::filesystem::path& targetDir);
		void MoveAsset(AssetHandle asset, const std::filesystem::path& targetDir);
		void MoveAssetInRegistry(const std::filesystem::path& sourcePath, const std::filesystem::path& targetPath);
		void MoveFullFolder(const std::filesystem::path& sourceDir, const std::filesystem::path& targetDir);

		void RenameAsset(AssetHandle asset, const std::string& newName);
		void RenameAssetFolder(AssetHandle asset, const std::filesystem::path& targetFilePath);

		void RemoveAsset(AssetHandle asset);
		void RemoveAsset(const std::filesystem::path& path);

		void RemoveFromRegistry(AssetHandle asset);
		void RemoveFromRegistry(const std::filesystem::path& path);
		void RemoveFullFolderFromRegistry(const std::filesystem::path& path);
		const AssetHandle AddAssetToRegistry(const std::filesystem::path& path, AssetHandle handle = 0);

		void ReloadAsset(AssetHandle handle);
		void ReloadAsset(const std::filesystem::path& path);

		bool IsSourceFile(AssetHandle handle) const;
		static bool IsLoaded(AssetHandle handle);

		static bool IsEngineAsset(const std::filesystem::path& path);
		static bool ExistsInRegistry(AssetHandle handle);
		static bool ExistsInRegistry(const std::filesystem::path& path);

		Ref<Asset> GetAssetRaw(AssetHandle assetHandle);
		Ref<Asset> QueueAssetRaw(AssetHandle assetHandle);

		static void SaveAsset(Ref<Asset> asset);
		static void SaveAssetAs(Ref<Asset> asset, const std::filesystem::path& targetFilePath);

		static const std::filesystem::path GetFilesystemPath(AssetHandle handle);
		static const std::filesystem::path GetFilesystemPath(const std::filesystem::path& path);
		static const std::filesystem::path GetRelativePath(const std::filesystem::path& path);
		static const std::filesystem::path GetFilePathFromAssetHandle(AssetHandle handle);
		static const std::filesystem::path GetContextPath(const std::filesystem::path& path);

		static AssetType GetAssetTypeFromHandle(const AssetHandle& handle);
		static AssetType GetAssetTypeFromPath(const std::filesystem::path& path);
		static AssetType GetAssetTypeFromExtension(const std::string& extension);
		static AssetHandle GetAssetHandleFromFilePath(const std::filesystem::path& path);
		
		static const AssetMetadata& GetMetadataFromHandle(AssetHandle handle);
		static const AssetMetadata& GetMetadataFromFilePath(const std::filesystem::path filePath);

		static const std::unordered_map<AssetHandle, AssetMetadata>& GetAssetRegistry();

		static std::string GetExtensionFromAssetType(AssetType type);

		// Is not guaranteed to return the "correct" asset if there are multiple assets with the same name and type
		static const std::filesystem::path GetFilePathFromFilename(const std::string& filename);

		[[nodiscard]] inline static AssetManager& Get() { return *s_instance; }

		template<typename T>
		static Ref<T> GetAsset(AssetHandle assetHandle);

		template<typename T>
		static Ref<T> GetAsset(const std::filesystem::path& path);

		template<typename T>
		static Ref<T> GetAssetLocking(AssetHandle assetHandle);

		template<typename T>
		static Ref<T> GetAssetLocking(const std::filesystem::path& path);

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
		static const std::vector<AssetHandle> GetAllAssetsOfType();

		static const std::vector<AssetHandle> GetAllAssetsOfType(AssetType assetType);
		static const std::vector<AssetHandle> GetAllAssetsWithDependency(const std::filesystem::path& dependencyPath);

	private:
		inline static AssetManager* s_instance = nullptr;
		inline static AssetMetadata s_nullMetadata = {};

		void LoadAsset(AssetHandle assetHandle, Ref<Asset>& asset);
		void DeserializeAssetMetaFile(std::filesystem::path metaPath);
		void LoadAssetMetaFiles();

		void SerializeAssetMetaFile(AssetHandle assetHandle);
		void RemoveMetaFile(const std::filesystem::path& filePath);

		void QueueAssetInternal(AssetHandle assetHandle, Ref<Asset>& asset);
		void QueueAssetInternal(AssetHandle assetHandle, Ref<Asset>& asset, const std::function<void()>& loadedCallback);

		static AssetMetadata& GetMetadataFromHandleMutable(AssetHandle handle);
		static AssetMetadata& GetMetadataFromFilePathMutable(const std::filesystem::path filePath);

		static const std::filesystem::path GetCleanAssetFilePath(const std::filesystem::path& path);
		std::vector<std::filesystem::path> GetMetaFiles();

		std::unordered_map<AssetType, Scope<AssetImporter>> m_assetImporters;
		std::unordered_map<AssetHandle, Ref<Asset>> m_assetCache;
		std::unordered_map<AssetHandle, Ref<Asset>> m_memoryAssets;
		std::unordered_map<AssetHandle, AssetMetadata> m_assetRegistry;

		mutable std::shared_mutex m_assetRegistryMutex;
		mutable std::shared_mutex m_assetCacheMutex;
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
		return GetAsset<T>(GetAssetHandleFromFilePath(path));
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
		return GetAsset<T>(GetAssetHandleFromFilePath(path));
	}

	template<typename T>
	inline Ref<T> AssetManager::QueueAsset(AssetHandle handle)
	{
		if (handle == Asset::Null())
		{
			return nullptr;
		}

		// If it's a memory asset, return it
		if (Get().m_memoryAssets.contains(handle))
		{
			return std::reinterpret_pointer_cast<T>(Get().m_memoryAssets.at(handle));
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
		if (Get().m_memoryAssets.contains(handle))
		{
			return std::reinterpret_pointer_cast<T>(Get().m_memoryAssets.at(handle));
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
		const std::filesystem::path filePath = Utils::ReplaceCharacter((targetDir / filename).string(), '\\', '/');

		WriteLock lockCache{ Get().m_assetCacheMutex };
		WriteLock lockRegistry{ Get().m_assetRegistryMutex };

		AssetMetadata metadata{};
		metadata.filePath = filePath;
		metadata.handle = asset->handle;
		metadata.type = T::GetStaticType();
		metadata.isLoaded = true;

		asset->name = metadata.filePath.stem().string();

		AssetManager::Get().m_assetRegistry.emplace(asset->handle, metadata);
		AssetManager::Get().m_assetCache.emplace(asset->handle, asset);

		Get().SerializeAssetMetaFile(metadata.handle);

		return asset;
	}

	template<typename T, typename ...Args>
	inline Ref<T> AssetManager::CreateMemoryAsset(Args&& ...args)
	{
		Ref<T> asset = CreateRef<T>(std::forward<Args>(args)...);
		
		AssetMetadata metadata{};
		metadata.filePath = "";
		metadata.handle = asset->handle;
		metadata.type = T::GetStaticType();
		metadata.isLoaded = true;
		metadata.isMemoryAsset = true;

		WriteLock lockCache{ Get().m_assetCacheMutex };
		WriteLock lockRegistry{ Get().m_assetRegistryMutex };

		AssetManager::Get().m_memoryAssets.emplace(asset->handle, asset);
		AssetManager::Get().m_assetRegistry.emplace(asset->handle, metadata);
		return asset;
	}

	template<typename ImporterType, typename Type>
	inline const ImporterType& AssetManager::GetImporterForType()
	{
		const auto type = Type::GetStaticType();
		if (!Get().m_assetImporters.contains(type))
		{
			VT_CORE_ASSERT(false, "Importer for type does not exist!");
		}

		return (ImporterType&)*Get().m_assetImporters.at(type);
	}
	template<typename T>
	inline const std::vector<Ref<T>> AssetManager::GetAllCachedAssetsOfType()
	{
		ReadLock lock{ Get().m_assetCacheMutex };

		std::vector<Ref<T>> result{};

		for (const auto& [handle, asset] : Get().m_assetCache)
		{
			if (asset->GetType() == T::GetStaticType())
			{
				result.push_back(reinterpret_pointer_cast<T>(asset));
			}
		}

		return result;
	}

	template<typename T>
	inline const std::vector<AssetHandle> AssetManager::GetAllAssetsOfType()
	{
		return GetAllAssetsOfType(T::GetStaticType());
	}
}
