#pragma once

#include "AssetSystem/Asset.h"

#include <LogModule/Log.h>

#include <SubSystem/SubSystem.h>

#include <CoreUtilities/Containers/Map.h>
#include <CoreUtilities/StringUtility.h>

#include <filesystem>
#include <unordered_map>
#include <shared_mutex>
#include <functional>

namespace Volt
{
	VT_DECLARE_LOG_CATEGORY_EXPORT(VTAS_API, LogAssetSystem, LogVerbosity::Trace);

	class AssetFactory;
	class AssetSerializer;
	class AssetDependencyGraph;

	class VTAS_API AssetManager
	{
	public:
		using WriteLock = std::unique_lock<std::shared_mutex>;
		using ReadLock = std::shared_lock<std::shared_mutex>;
		using AssetCreateFunction = std::function<Ref<Asset>()>;
		using AssetChangedCallback = std::function<void(AssetHandle assetHandle, AssetChangedState state)>;

		using AssetRegistry = vt::map<AssetHandle, AssetMetadata>;
		using AssetCache = vt::map<AssetHandle, Ref<Asset>>;

		AssetManager(const std::filesystem::path& projectDirectory, const std::filesystem::path& assetsDirectory, const std::filesystem::path& engineDirectory);
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

		void AddAssetToRegistry(const std::filesystem::path& path, AssetHandle handle, AssetType type);
		AssetHandle GetOrAddAssetToRegistry(const std::filesystem::path& path, AssetType type);

		void ReloadAsset(AssetHandle handle);
		void ReloadAsset(const std::filesystem::path& path);

		Ref<Asset> GetAssetRaw(AssetHandle assetHandle);
		Ref<Asset> QueueAssetRaw(AssetHandle assetHandle);

		static void Update();

		static UUID64 RegisterAssetChangedCallback(AssetType assetType, AssetChangedCallback&& callbackFunction);
		static void UnregisterAssetChangedCallback(AssetType assetType, UUID64 id);

		static void AddDependencyToAsset(AssetHandle handle, AssetHandle dependency);
		static Vector<AssetHandle> GetAssetsDependentOn(AssetHandle handle);

		static bool IsLoaded(AssetHandle handle);

		static bool IsEngineAsset(const std::filesystem::path& path);
		static bool IsMemoryAsset(AssetHandle handle);
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
		static AssetHandle GetAssetHandleFromFilePath(const std::filesystem::path& path);
		
		static const AssetMetadata& GetMetadataFromHandle(AssetHandle handle);
		static const AssetMetadata& GetMetadataFromFilePath(const std::filesystem::path filePath);

		static const AssetRegistry& GetAssetRegistry();
		static AssetRegistry& GetAssetRegistryMutable();

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
		static const Vector<Ref<T>> GetAllCachedAssetsOfType();

		template<typename T>
		static const Vector<AssetHandle> GetAllAssetsOfType();

		static const Vector<AssetHandle> GetAllAssetsOfType(AssetType assetType);

	private:
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

		inline static AssetManager* s_instance = nullptr;
		inline static AssetMetadata s_nullMetadata = {};

		void UpdateInternal();

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
		
		Vector<std::filesystem::path> GetEngineAssetFiles();
		Vector<std::filesystem::path> GetProjectAssetFiles();

		AssetCache m_assetCache;
		AssetCache m_memoryAssets;
		AssetRegistry m_assetRegistry;

		std::filesystem::path m_projectDirectory;
		std::filesystem::path m_assetsDirectory;
		std::filesystem::path m_engineDirectory;

		std::unordered_map<AssetType, Vector<AssetChangedCallbackInfo>> m_assetChangedCallbacks;
		Vector<AssetChangedQueueInfo> m_assetChangedQueue;
		std::mutex m_assetChangedQueueMutex;
		Scope<AssetDependencyGraph> m_dependencyGraph;

		std::mutex m_assetCallbackMutex;
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
				VT_LOGC(Error, LogAssetSystem, "Trying to load asset {} which has invalid metadata!", assetHandle);
				return nullptr;
			}
		}

		Ref<Asset> asset = CreateRef<T>();
		if (!ValidateAssetType(assetHandle, asset))
		{
			VT_LOGC(Critical, LogAssetSystem, "Asset type does not match!");
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
				VT_LOGC(Error, LogAssetSystem, "Trying to load asset which has invalid metadata!");
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
				VT_LOGC(Error, LogAssetSystem, "Trying to load asset which has invalid metadata!");
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
			VT_LOGC(Critical, LogAssetSystem, "Asset type does not match!");
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

		const std::string fileExtension = ".vtasset";
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
		VT_ASSERT_MSG(Get().m_assetSerializers.contains(type), "Importer for type does not exist!");

		return (ImporterType&)*Get().m_assetSerializers.at(type);
	}
	template<typename T>
	inline const Vector<Ref<T>> AssetManager::GetAllCachedAssetsOfType()
	{
		ReadLock lock{ Get().m_assetCacheMutex };

		Vector<Ref<T>> result{};

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
	inline const Vector<AssetHandle> AssetManager::GetAllAssetsOfType()
	{
		return GetAllAssetsOfType(T::GetStaticType());
	}
}
