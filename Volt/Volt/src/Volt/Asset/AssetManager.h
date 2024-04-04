#pragma once

#include "Volt/Asset/Asset.h"

#include "Volt/Core/Base.h"
#include "Volt/Core/Profiling.h"
#include "Volt/Log/Log.h"

#include "Volt/Project/ProjectManager.h"
#include "Volt/Utility/StringUtility.h"

#include <map>
#include <filesystem>
#include <unordered_map>
#include <shared_mutex>
#include <functional>

namespace Volt
{
	class AssetFactory;
	class AssetSerializer;
	class AssetDependencyGraph;
	class AssetManager
	{
	public:
		using WriteLock = std::unique_lock<std::shared_mutex>;
		using ReadLock = std::shared_lock<std::shared_mutex>;
		using AssetCreateFunction = std::function<Ref<Asset>()>;
		using AssetChangedCallback = std::function<void(AssetHandle assetHandle, AssetChangedState state)>;

		AssetManager();
		~AssetManager();

		void Initialize();
		void Shutdown();

		void Unload(AssetHandle assetHandle);

		void MoveAsset(Ref<Asset> asset, const std::filesystem::path& targetDir);
		void MoveAsset(AssetHandle asset, const std::filesystem::path& targetDir);
		void MoveAssetInRegistry(const std::filesystem::path& sourcePath, const std::filesystem::path& targetPath);
		void MoveFullFolder(const std::filesystem::path& sourceDir, const std::filesystem::path& targetDir);

		void RenameAsset(AssetHandle asset, const std::string& newName);
		void RenameAssetFolder(AssetHandle asset, const std::filesystem::path& targetFilePath);

		void RemoveAsset(AssetHandle asset);
		void RemoveAsset(const std::filesystem::path& path);

		void RemoveAssetFromRegistry(AssetHandle asset);
		void RemoveAssetFromRegistry(const std::filesystem::path& path);
		void RemoveFullFolderFromRegistry(const std::filesystem::path& path);

		const AssetHandle AddAssetToRegistry(const std::filesystem::path& path, AssetHandle handle = 0);
		void AddAssetToRegistry(const std::filesystem::path& path, AssetHandle handle, AssetType type);

		void ReloadAsset(AssetHandle handle);
		void ReloadAsset(const std::filesystem::path& path);

		inline const AssetFactory& GetFactory() const { return *m_assetFactory; }

		static bool IsSourceAsset(AssetType type);
		static bool IsSourceAsset(AssetHandle handle);
		Ref<Asset> GetAssetRaw(AssetHandle assetHandle);
		Ref<Asset> QueueAssetRaw(AssetHandle assetHandle);

		static void Update();

		static UUID64 RegisterAssetChangedCallback(AssetType assetType, AssetChangedCallback&& callbackFunction);
		static void UnregisterAssetChangedCallback(AssetType assetType, UUID64 id);

		static void AddDependencyToAsset(AssetHandle handle, AssetHandle dependency);

		static bool IsLoaded(AssetHandle handle);

		static bool IsEngineAsset(const std::filesystem::path& path);
		static bool ExistsInRegistry(AssetHandle handle);
		static bool ExistsInRegistry(const std::filesystem::path& path);

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
		static std::unordered_map<AssetHandle, AssetMetadata>& GetAssetRegistryMutable();

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

		template<typename T, typename... Args>
		static Ref<T> CreateAsset(const std::filesystem::path& targetDir, const std::string& name, Args&&... args);

		template<typename T, typename... Args>
		static Ref<T> CreateMemoryAsset(const std::string& name, Args&&... args);

		template<typename ImporterType, typename Type>
		static const ImporterType& GetImporterForType();

		template<typename T>
		static const std::vector<Ref<T>> GetAllCachedAssetsOfType();

		template<typename T>
		static const std::vector<AssetHandle> GetAllAssetsOfType();

		static const std::vector<AssetHandle> GetAllAssetsOfType(AssetType assetType);

	private:
		inline static AssetManager* s_instance = nullptr;
		inline static AssetMetadata s_nullMetadata = {};

		void UpdateInternal();
		void RegisterAssetSerializers();

		void LoadAsset(AssetHandle assetHandle, Ref<Asset>& asset);

		void LoadAllAssetMetadata();
		void DeserializeAssetMetadata(std::filesystem::path assetPath);

		void OnAssetChanged(AssetHandle assetHandle, AssetChangedState state);
		void QueueAssetChanged(AssetHandle assetHandle, AssetChangedState state);

		void QueueAssetInternal(AssetHandle assetHandle, Ref<Asset>& asset);

		static bool ValidateAssetType(AssetHandle handle, Ref<Asset> asset);
		static AssetMetadata& GetMetadataFromHandleMutable(AssetHandle handle);
		static AssetMetadata& GetMetadataFromFilePathMutable(const std::filesystem::path filePath);

		static const std::filesystem::path GetCleanAssetFilePath(const std::filesystem::path& path);
		
		std::vector<std::filesystem::path> GetEngineAssetFiles();
		std::vector<std::filesystem::path> GetProjectAssetFiles();

		std::unordered_map<AssetType, Scope<AssetSerializer>> m_assetSerializers;
		std::unordered_map<AssetType, AssetCreateFunction> m_assetCreateFunctions;

		std::unordered_map<AssetHandle, Ref<Asset>> m_assetCache;
		std::unordered_map<AssetHandle, Ref<Asset>> m_memoryAssets;
		std::unordered_map<AssetHandle, AssetMetadata> m_assetRegistry;

		struct AssetChangedCallbackInfo
		{
			UUID64 id;
			AssetChangedCallback callback;
		};

		struct AssetChangedQueueInfo
		{
			AssetHandle handle;
			AssetChangedState state;
		};

		std::unordered_map<AssetType, std::vector<AssetChangedCallbackInfo>> m_assetChangedCallbacks;
		std::vector<AssetChangedQueueInfo> m_assetChangedQueue;
		std::mutex m_assetChangedQueueMutex;
		Scope<AssetFactory> m_assetFactory;
		Scope<AssetDependencyGraph> m_dependencyGraph;

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

		{
			ReadLock lock{ Get().m_assetRegistryMutex };
			const auto& metadata = GetMetadataFromHandle(assetHandle);
			if (!metadata.IsValid())
			{
				VT_CORE_ERROR("[AssetManager] Trying to load asset which has invalid metadata!");
				return nullptr;
			}
		}

		Ref<Asset> asset = CreateRef<T>();
		if (!ValidateAssetType(assetHandle, asset))
		{
			VT_CORE_CRITICAL("[AssetManager] Asset type does not match!");
			return nullptr;
		}

		Get().LoadAsset(assetHandle, asset);
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

		{
			ReadLock lock{ Get().m_assetRegistryMutex };
			const auto& metadata = GetMetadataFromHandle(assetHandle);
			if (!metadata.IsValid())
			{
				VT_CORE_ERROR("[AssetManager] Trying to load asset which has invalid metadata!");
				return nullptr;
			}
		}

		Ref<Asset> asset = QueueAsset<T>(assetHandle);

		if (!asset)
		{
			return nullptr;
		}

		while (!asset->IsValid())
		{}
		return std::reinterpret_pointer_cast<T>(asset);
	}

	template<typename T>
	inline Ref<T> AssetManager::GetAssetLocking(const std::filesystem::path& path)
	{
		return GetAssetLocking<T>(GetAssetHandleFromFilePath(path));
	}

	template<typename T>
	inline Ref<T> AssetManager::QueueAsset(AssetHandle handle)
	{
		if (handle == Asset::Null())
		{
			return nullptr;
		}

		{
			ReadLock lock{ Get().m_assetRegistryMutex };
			const auto& metadata = GetMetadataFromHandle(handle);
			if (!metadata.IsValid())
			{
				VT_CORE_ERROR("[AssetManager] Trying to load asset which has invalid metadata!");
				return nullptr;
			}
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
		if (!ValidateAssetType(handle, asset))
		{
			VT_CORE_CRITICAL("[AssetManager] Asset type does not match!");
			return nullptr;
		}

		asset->SetFlag(AssetFlag::Queued, true);
		Get().QueueAssetInternal(handle, asset);

		return std::reinterpret_pointer_cast<T>(asset);
	}

	template<typename T, typename ...Args>
	inline Ref<T> AssetManager::CreateAsset(const std::filesystem::path& targetDir, const std::string& name, Args && ...args)
	{
		Ref<T> asset = CreateRef<T>(std::forward<Args>(args)...);

		//#TODO_Ivar: Move to clean name function
		std::string cleanName = name;
		cleanName.erase(std::remove_if(cleanName.begin(), cleanName.end(), [](char c) { return c == ':'; }), cleanName.end());

		const auto& fileExtension = GetExtensionFromAssetType(T::GetStaticType());
		const std::filesystem::path filePath = ::Utility::ReplaceCharacter((targetDir / (cleanName + fileExtension)).string(), '\\', '/');

		WriteLock lockCache{ Get().m_assetCacheMutex };
		WriteLock lockRegistry{ Get().m_assetRegistryMutex };

		AssetMetadata metadata{};
		metadata.filePath = filePath;
		metadata.handle = asset->handle;
		metadata.type = T::GetStaticType();
		metadata.isLoaded = true;

		asset->assetName = cleanName;

		AssetManager::Get().m_assetRegistry.emplace(asset->handle, metadata);
		AssetManager::Get().m_assetCache.emplace(asset->handle, asset);

		return asset;
	}

	template<typename T, typename ...Args>
	inline Ref<T> AssetManager::CreateMemoryAsset(const std::string& name, Args&& ...args)
	{
		Ref<T> asset = CreateRef<T>(std::forward<Args>(args)...);
		asset->assetName = name;

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
		if (!Get().m_assetSerializers.contains(type))
		{
			VT_CORE_ASSERT(false, "Importer for type does not exist!");
		}

		return (ImporterType&)*Get().m_assetSerializers.at(type);
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
