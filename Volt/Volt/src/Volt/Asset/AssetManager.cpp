#include "vtpch.h"
#include "AssetManager.h"

#include "Volt/Core/Base.h"
#include "Volt/Core/Application.h"

#include "Volt/Asset/AssetDependencyGraph.h"
#include "Volt/Asset/Importers/MeshTypeImporter.h"
#include "Volt/Asset/Importers/TextureImporter.h"

#include "Volt/Asset/Serializers/AnimatedCharacterSerializer.h"
#include "Volt/Asset/Serializers/AnimationSerializer.h"
#include "Volt/Asset/Serializers/BehaviourTreeSerializer.h"
#include "Volt/Asset/Serializers/BlendSpaceSerializer.h"
#include "Volt/Asset/Serializers/FontSerializer.h"
#include "Volt/Asset/Serializers/MaterialSerializer.h"
#include "Volt/Asset/Serializers/MeshSerializer.h"
#include "Volt/Asset/Serializers/NavigationMeshSerializer.h"
#include "Volt/Asset/Serializers/NetContractSerializer.h"
#include "Volt/Asset/Serializers/ParticlePresetSerializer.h"
#include "Volt/Asset/Serializers/PhysicsMaterialSerializer.h"
#include "Volt/Asset/Serializers/PrefabSerializer.h"
#include "Volt/Asset/Serializers/SceneSerializer.h"
#include "Volt/Asset/Serializers/ShaderDefinitionSerializer.h"
#include "Volt/Asset/Serializers/SkeletonSerializer.h"
#include "Volt/Asset/Serializers/SourceMeshSerializer.h"
#include "Volt/Asset/Serializers/SourceTextureSerializer.h"
#include "Volt/Asset/Serializers/TextureSerializer.h"
#include "Volt/Asset/Serializers/MotionWeaveDatabaseSerializer.h"
#include "Volt/Asset/AssetFactory.h"

#include "Volt/Net/SceneInteraction/NetContract.h"

#include "Volt/Utility/FileSystem.h"
#include "Volt/Utility/YAMLSerializationHelpers.h"

#include <CoreUtilities/ThreadUtilities.h>
#include <CoreUtilities/Time/ScopedTimer.h>

namespace Volt
{
	AssetManager::AssetManager()
	{
		VT_ASSERT_MSG(!s_instance, "AssetManager already exists!");
		s_instance = this;

		Initialize();
	}

	AssetManager::~AssetManager()
	{
		Shutdown();
		s_instance = nullptr;
	}

	void AssetManager::Initialize()
	{
		MeshTypeImporter::Initialize();
		TextureImporter::Initialize();

		m_dependencyGraph = CreateScope<AssetDependencyGraph>();

		m_assetFactory = CreateScope<AssetFactory>();
		m_assetFactory->Initialize();

		if (!ProjectManager::AreCurrentProjectMetaFilesDeprecated())
		{
			NetContractContainer::Load();
		}

		RegisterAssetSerializers();
		LoadAllAssetMetadata();
	}

	void AssetManager::Shutdown()
	{
		m_assetCache.clear();
		m_memoryAssets.clear();
		m_assetRegistry.clear();

		m_assetFactory->Shutdown();
		m_assetFactory = nullptr;
		m_dependencyGraph = nullptr;

		TextureImporter::Shutdown();
		MeshTypeImporter::Shutdown();
	}

	UUID64 AssetManager::RegisterAssetChangedCallback(AssetType assetType, AssetChangedCallback&& callbackFunction)
	{
		AssetManager& instance = Get();
		std::scoped_lock lock{ instance.m_assetCallbackMutex };
		
		UUID64 id = UUID64{};

		instance.m_assetChangedCallbacks[assetType].push_back({ id, callbackFunction });
		return id;
	}

	void AssetManager::UnregisterAssetChangedCallback(AssetType assetType, UUID64 id)
	{
		AssetManager& instance = Get();
		std::scoped_lock lock{ instance.m_assetCallbackMutex };

		auto& callbacks = instance.m_assetChangedCallbacks[assetType];

		auto it = std::find_if(callbacks.begin(), callbacks.end(), [&](const AssetChangedCallbackInfo& callbackInfo) 
		{
			return callbackInfo.id == id;
		}); 

		if (it != callbacks.end())
		{
			callbacks.erase(it);
		}
	}

	const Vector<AssetHandle> AssetManager::GetAllAssetsOfType(AssetType wantedAssetType)
	{
		auto& instance = Get();

		ReadLock lock{ instance.m_assetRegistryMutex };
		Vector<AssetHandle> result;

		for (const auto& [handle, metadata] : instance.m_assetRegistry)
		{
			if (metadata.type == wantedAssetType || wantedAssetType == AssetType::None)
			{
				result.emplace_back(handle);
			}
		}

		return result;
	}

	void AssetManager::RegisterAssetSerializers()
	{
		m_assetSerializers.emplace(AssetType::MeshSource, CreateScope<SourceMeshSerializer>());
		m_assetSerializers.emplace(AssetType::TextureSource, CreateScope<SourceTextureSerializer>());
		m_assetSerializers.emplace(AssetType::Texture, CreateScope<TextureSerializer>());
		m_assetSerializers.emplace(AssetType::ShaderDefinition, CreateScope<ShaderDefinitionSerializer>());
		m_assetSerializers.emplace(AssetType::Material, CreateScope<MaterialSerializer>());
		m_assetSerializers.emplace(AssetType::Mesh, CreateScope<MeshSerializer>());
		m_assetSerializers.emplace(AssetType::NavMesh, CreateScope<NavigationMeshSerializer>());
		m_assetSerializers.emplace(AssetType::Scene, CreateScope<SceneSerializer>());
		m_assetSerializers.emplace(AssetType::Skeleton, CreateScope<SkeletonSerializer>());
		m_assetSerializers.emplace(AssetType::Animation, CreateScope<AnimationSerializer>());
		m_assetSerializers.emplace(AssetType::AnimatedCharacter, CreateScope<AnimatedCharacterSerializer>());
		m_assetSerializers.emplace(AssetType::ParticlePreset, CreateScope<ParticlePresetSerializer>());
		m_assetSerializers.emplace(AssetType::Prefab, CreateScope<PrefabSerializer>());
		m_assetSerializers.emplace(AssetType::Font, CreateScope<FontSerializer>());
		m_assetSerializers.emplace(AssetType::PhysicsMaterial, CreateScope<PhysicsMaterialSerializer>());
		m_assetSerializers.emplace(AssetType::BehaviorGraph, CreateScope<BehaviourTreeSerializer>());
		m_assetSerializers.emplace(AssetType::BlendSpace, CreateScope<BlendSpaceSerializer>());
		m_assetSerializers.emplace(AssetType::NetContract, CreateScope<NetContractSerializer>());
		m_assetSerializers.emplace(AssetType::MotionWeave, CreateScope<MotionWeaveDatabaseSerializer>());
	}

	void AssetManager::UpdateInternal()
	{
		{
			std::scoped_lock lock{ m_assetChangedQueueMutex };
			if (!m_assetChangedQueue.empty())
			{
				for (const auto& info : m_assetChangedQueue)
				{
					OnAssetChanged(info.handle, info.state);
				}

				m_assetChangedQueue.clear();
			}
		}
	}

	void AssetManager::LoadAsset(AssetHandle assetHandle, Ref<Asset>& asset)
	{
		{
			ReadLock lock{ m_assetCacheMutex };
			if (m_assetCache.contains(assetHandle))
			{
				asset = m_assetCache.at(assetHandle);
				return;
			}
		}

		if (m_memoryAssets.contains(assetHandle))
		{
			asset = m_memoryAssets.at(assetHandle);
			return;
		}

		AssetMetadata metadata;

		{
			ReadLock lock{ m_assetRegistryMutex };
			metadata = GetMetadataFromHandle(assetHandle);
		}

		if (!metadata.IsValid())
		{
			VT_LOG(LogVerbosity::Error, "[AssetManager] Trying to load asset which has invalid metadata!");
			asset->SetFlag(AssetFlag::Invalid, true);
			return;
		}

		if (!m_assetSerializers.contains(metadata.type))
		{
			VT_LOG(LogVerbosity::Warning, "[AssetManager] No importer for asset found!");
			asset->SetFlag(AssetFlag::Invalid, true);
			return;
		}

		m_dependencyGraph->AddAssetToGraph(assetHandle);

		asset->handle = metadata.handle;
		asset->assetName = metadata.filePath.stem().string();

		{
#ifndef VT_DIST
			ScopedTimer timer{};
#endif
			m_assetSerializers.at(metadata.type)->Deserialize(metadata, asset);

#ifndef VT_DIST
			VT_LOG(LogVerbosity::Trace, "[AssetManager]: Loaded asset {0} with handle {1} in {2} seconds!", metadata.filePath, asset->handle, timer.GetTime<Time::Seconds>());
#endif	
		}

		{
			WriteLock lock{ m_assetRegistryMutex };
			AssetMetadata& postMeta = GetMetadataFromHandleMutable(assetHandle);
			postMeta.isLoaded = true;
		}

		m_dependencyGraph->OnAssetChanged(assetHandle, AssetChangedState::Updated);

		{
			WriteLock lock{ m_assetCacheMutex };
			m_assetCache.emplace(asset->handle, asset);
		}
	}

	void AssetManager::LoadAllAssetMetadata()
	{
		VT_LOG(LogVerbosity::Info, "[AssetManager] Fetching asset meta data...");
		ScopedTimer timer{};

		const auto projectAssetFiles = GetProjectAssetFiles();
		const auto engineAssetFiles = GetEngineAssetFiles();

		for (auto file : engineAssetFiles)
		{
			DeserializeAssetMetadata(file);
		}

		for (auto file : projectAssetFiles)
		{
			DeserializeAssetMetadata(GetFilesystemPath(file));
		}

		for (const auto& [handle, metadata] : m_assetRegistry)
		{
			m_dependencyGraph->AddAssetToGraph(handle);
		}

		VT_LOG(LogVerbosity::Info, "[AssetManager] Finished fetching meta data in {} seconds!", timer.GetTime<Time::Seconds>());
	}

	void AssetManager::DeserializeAssetMetadata(std::filesystem::path assetPath)
	{
		constexpr size_t assetHeaderSize = sizeof(SerializedAssetMetadata) + sizeof(uint32_t) + sizeof(TypeHeader) * 6;

		BinaryStreamReader streamReader{ assetPath, assetHeaderSize };
		if (!streamReader.IsStreamValid())
		{
			VT_LOG(LogVerbosity::Error, "Failed to open file: {0}!", assetPath);
			return;
		}

		uint32_t value = 0;
		bool couldReadValue = streamReader.TryRead(value);
		if (!couldReadValue || value != SerializedAssetMetadata::AssetMagic)
		{
			return;
		}

		streamReader.ResetHead();

		SerializedAssetMetadata serializedMetadata = AssetSerializer::ReadMetadata(streamReader);

		{
			WriteLock lock{ m_assetRegistryMutex };
			AssetMetadata& metadata = m_assetRegistry[serializedMetadata.handle];
			metadata.handle = serializedMetadata.handle;
			metadata.filePath = GetRelativePath(assetPath);
			metadata.type = serializedMetadata.type;
		}
	}

	void AssetManager::Unload(AssetHandle assetHandle)
	{
		{
			ReadLock lock{ m_assetCacheMutex };
			if (!m_assetCache.contains(assetHandle))
			{
				VT_LOG(LogVerbosity::Warning, "[AssetManager] Unable to unload asset with handle {0}, it has not been loaded!", assetHandle);
				return;
			}
		}

		{
			WriteLock lock{ m_assetRegistryMutex };
			if (!m_assetRegistry.contains(assetHandle))
			{
				VT_LOG(LogVerbosity::Warning, "[AssetManager] Unable to unload asset with handle {0}, it does not exist in the registry!", assetHandle);
				return;
			}

			m_assetRegistry.at(assetHandle).isLoaded = false;
		}

		{
			WriteLock lock{ m_assetCacheMutex };
			m_assetCache.erase(assetHandle);
		}
	}

	void AssetManager::ReloadAsset(const std::filesystem::path& path)
	{
		AssetHandle handle = GetAssetHandleFromFilePath(path);
		if (handle == Asset::Null())
		{
			VT_LOG(LogVerbosity::Error, "[AssetManager] Asset with path {0} is not loaded!", path.string());
			return;
		}

		ReloadAsset(handle);
	}

	void AssetManager::ReloadAsset(AssetHandle handle)
	{
		Unload(handle);

		const auto type = GetAssetTypeFromHandle(handle);
		if (type == AssetType::None)
		{
			return;
		}

		Ref<Asset> asset = m_assetFactory->CreateAssetOfType(type);
		LoadAsset(handle, asset);
	}

	// #TODO_Ivar: This function does not seem to do what it's supposed to... (should we not create a new asset..?)
	void AssetManager::SaveAssetAs(Ref<Asset> asset, const std::filesystem::path& targetFilePath)
	{
		auto& instance = Get();

		if (instance.m_assetSerializers.find(asset->GetType()) == instance.m_assetSerializers.end())
		{
			VT_LOG(LogVerbosity::Error, "[AssetManager] No exporter for asset {0} found!", asset->handle);
			return;
		}

		if (!asset->IsValid())
		{
			VT_LOG(LogVerbosity::Error, "[AssetManager] Unable to save invalid asset {0}!", asset->handle);
			return;
		}

		// If the asset already exists in the registry, we only update the file path
		if (!instance.m_assetRegistry.contains(asset->handle))
		{
			AssetMetadata& metaData = instance.m_assetRegistry[asset->handle];
			metaData.filePath = GetCleanAssetFilePath(targetFilePath);
			metaData.handle = asset->handle;
			metaData.isLoaded = true;
			metaData.type = asset->GetType();
		}
		else
		{
			WriteLock lock{ instance.m_assetRegistryMutex };
			AssetMetadata& metaData = instance.m_assetRegistry[asset->handle];
			metaData.filePath = GetCleanAssetFilePath(targetFilePath);
		}

		AssetMetadata metadata = s_nullMetadata;

		{
			ReadLock lock{ instance.m_assetRegistryMutex };
			metadata = GetMetadataFromHandle(asset->handle);
			asset->assetName = metadata.filePath.stem().string();
		}

		{
#ifndef VT_DIST
			ScopedTimer timer{};
#endif

			instance.m_assetSerializers[metadata.type]->Serialize(metadata, asset);

#ifndef VT_DIST
			VT_LOG(LogVerbosity::Trace, "[AssetManager]: Saved asset {0} to {1} in {2} seconds!", metadata.handle, metadata.filePath, timer.GetTime<Time::Seconds>());
#endif
		}

		{
			WriteLock lock{ instance.m_assetCacheMutex };
			if (!instance.m_assetCache.contains(asset->handle))
			{
				instance.m_assetCache.emplace(asset->handle, asset);
			}
		}
	}

	void AssetManager::SaveAsset(const Ref<Asset> asset)
	{
		auto& instance = Get();

		if (instance.m_assetSerializers.find(asset->GetType()) == instance.m_assetSerializers.end())
		{
			VT_LOG(LogVerbosity::Error, "[AssetManager] No exporter for asset {0} found!", asset->handle);
			return;
		}

		if (!asset->IsValid())
		{
			VT_LOG(LogVerbosity::Error, "[AssetManager] Unable to save invalid asset {0}!", asset->handle);
			return;
		}

		AssetMetadata metadata = s_nullMetadata;

		{
			ReadLock lock{ instance.m_assetRegistryMutex };
			metadata = GetMetadataFromHandle(asset->handle);
		}

		{
#ifndef VT_DIST
			ScopedTimer timer{};
#endif

			instance.m_assetSerializers[metadata.type]->Serialize(metadata, asset);

#ifndef VT_DIST
			VT_LOG(LogVerbosity::Trace, "[AssetManager]: Saved asset {0} to {1} in {2} seconds!", metadata.handle, metadata.filePath, timer.GetTime<Time::Seconds>());
#endif
		}

		{
			WriteLock lock{ instance.m_assetCacheMutex };
			if (!instance.m_assetCache.contains(asset->handle))
			{
				instance.m_assetCache.emplace(asset->handle, asset);
			}
		}
	}

	void AssetManager::MoveAsset(Ref<Asset> asset, const std::filesystem::path& targetDir)
	{
		const auto projDir = GetContextPath(targetDir);

		std::filesystem::path assetFilePath;

		{
			ReadLock lock{ m_assetRegistryMutex };

			const auto& assetMetaData = GetMetadataFromHandle(asset->handle);
			if (!assetMetaData.IsValid())
			{
				VT_LOG(LogVerbosity::Warning, "[AssetMananger] Unable to move invalid asset {0}!", asset->handle);
				return;
			}

			assetFilePath = assetMetaData.filePath;
		}

		FileSystem::Move(projDir / assetFilePath, projDir / targetDir);

		const std::filesystem::path newPath = targetDir / assetFilePath.filename();

		{
			WriteLock lock{ m_assetRegistryMutex };
			m_assetRegistry[asset->handle].filePath = newPath;
		}
	}

	void AssetManager::MoveAsset(AssetHandle assetHandle, const std::filesystem::path& targetDir)
	{
		std::filesystem::path assetFilePath;

		{
			ReadLock lock{ m_assetRegistryMutex };

			const auto& assetMetaData = GetMetadataFromHandle(assetHandle);
			if (!assetMetaData.IsValid())
			{
				VT_LOG(LogVerbosity::Warning, "[AssetMananger] Unable to move invalid asset {0}!", assetHandle);
				return;
			}

			assetFilePath = assetMetaData.filePath;
		}

		const std::filesystem::path newPath = targetDir / assetFilePath.filename();
		const auto projDir = GetContextPath(targetDir);

		FileSystem::Move(projDir / assetFilePath, projDir / targetDir);

		{
			WriteLock lock{ m_assetRegistryMutex };
			m_assetRegistry[assetHandle].filePath = newPath;
		}
	}

	void AssetManager::MoveAssetInRegistry(const std::filesystem::path& sourcePath, const std::filesystem::path& targetPath)
	{
		const auto projDir = GetContextPath(targetPath);

		AssetHandle assetHandle = Asset::Null();

		{
			WriteLock lock{ m_assetRegistryMutex };
			AssetMetadata& metadata = GetMetadataFromFilePathMutable(sourcePath);

			metadata.filePath = GetCleanAssetFilePath(targetPath);
			assetHandle = metadata.handle;
		}
	}

	void AssetManager::MoveFullFolder(const std::filesystem::path& sourceDir, const std::filesystem::path& targetDir)
	{
		if (sourceDir.empty())
		{
			VT_LOG(LogVerbosity::Warning, "[AssetManager] Trying to move invalid directory!");
			return;
		}

		if (targetDir.empty())
		{
			VT_LOG(LogVerbosity::Warning, "[AssetManager] Trying to move directory {0} to an invalid directory!");
			return;
		}

		Vector<AssetHandle> filesToMove{};
		{
			ReadLock lock{ m_assetRegistryMutex };
			const std::string sourceDirLower = ::Utility::ToLower(sourceDir.string());

			for (const auto& [handle, metaData] : m_assetRegistry)
			{
				const std::string filePathLower = ::Utility::ToLower(metaData.filePath.string());

				if (auto it = filePathLower.find(sourceDirLower); it != std::string::npos)
				{
					filesToMove.emplace_back(handle);
				}
			}
		}

		{
			WriteLock lock{ m_assetRegistryMutex };
			for (const auto& handle : filesToMove)
			{
				auto& metadata = GetMetadataFromHandleMutable(handle);

				std::string newPath = metadata.filePath.string();
				const size_t directoryStringLoc = ::Utility::ToLower(newPath).find(::Utility::ToLower(sourceDir.string()));

				if (directoryStringLoc == std::string::npos)
				{
					continue;
				}

				newPath.erase(directoryStringLoc, sourceDir.string().length());
				newPath.insert(directoryStringLoc, targetDir.string());
				metadata.filePath = GetCleanAssetFilePath(newPath);
			}
		}
	}

	void AssetManager::RenameAsset(AssetHandle assetHandle, const std::string& newName)
	{
		const std::filesystem::path oldPath = GetFilePathFromAssetHandle(assetHandle);
		const std::filesystem::path newPath = oldPath.parent_path() / (newName + oldPath.extension().string());
		const auto projDir = GetContextPath(oldPath);

		{
			WriteLock lock{ m_assetRegistryMutex };
			if (!m_assetRegistry.contains(assetHandle))
			{
				VT_LOG(LogVerbosity::Warning, "[AssetManager] Trying to rename invalid asset!");
				return;
			}

			m_assetRegistry.at(assetHandle).filePath = newPath;
		}

		FileSystem::Rename(projDir / oldPath, newName);
	}

	void AssetManager::RenameAssetFolder(AssetHandle assetHandle, const std::filesystem::path& targetFilePath)
	{
		WriteLock lock{ m_assetRegistryMutex };

		auto& metadata = GetMetadataFromHandleMutable(assetHandle);
		if (!metadata.IsValid())
		{
			VT_LOG(LogVerbosity::Warning, "[AssetManager] Trying to rename invalid asset {0}!", assetHandle);
			return;
		}

		metadata.filePath = GetCleanAssetFilePath(targetFilePath);
	}

	void AssetManager::RemoveAsset(AssetHandle assetHandle)
	{
		const auto metadata = GetMetadataFromHandle(assetHandle);
		if (!metadata.IsValid())
		{
			VT_LOG(LogVerbosity::Warning, "[AssetManager] Trying to remove invalid asset {0}!", assetHandle);
			return;
		}

		WriteLock lock{ m_assetRegistryMutex };
		m_assetRegistry.erase(assetHandle);

		const std::filesystem::path filePath = metadata.filePath;
		const auto projDir = GetContextPath(filePath);

		{
			WriteLock cacheLock{ m_assetCacheMutex };
			m_assetCache.erase(assetHandle);
		}

		m_dependencyGraph->OnAssetChanged(assetHandle, AssetChangedState::Removed);
		QueueAssetChanged(assetHandle, AssetChangedState::Removed);
		m_dependencyGraph->RemoveAssetFromGraph(assetHandle);

		FileSystem::MoveToRecycleBin(projDir / filePath);

#ifdef VT_DEBUG
		VT_LOG(LogVerbosity::Info, "[AssetManager] Removed asset {0} with handle {1}!", assetHandle, filePath.string());
#endif
	}

	void AssetManager::RemoveAsset(const std::filesystem::path& path)
	{
		RemoveAsset(GetAssetHandleFromFilePath(path));
	}

	bool AssetManager::ValidateAssetType(AssetHandle handle, Ref<Asset> asset)
	{
		ReadLock lock{ Get().m_assetRegistryMutex };
		const auto& metadata = GetMetadataFromHandle(handle);

		// If the metadata is not valid we allow the asset to be valid
		if (!metadata.IsValid())
		{
			return true;
		}

		VT_ASSERT_MSG(metadata.type == asset->GetType(), "Asset type does not match meta type!");

		return metadata.type == asset->GetType();
	}

	void AssetManager::OnAssetChanged(AssetHandle assetHandle, AssetChangedState state)
	{
		const auto type = GetAssetTypeFromHandle(assetHandle);
		if (m_assetChangedCallbacks.contains(type))
		{
			const auto& callbacks = m_assetChangedCallbacks.at(type);
			for (const auto& callback : callbacks)
			{
				if (!callback.callback)
				{
					continue;
				}

				callback.callback(assetHandle, state);
			}
		}
	}

	void AssetManager::QueueAssetChanged(AssetHandle assetHandle, AssetChangedState state)
	{
		std::scoped_lock lock{ m_assetChangedQueueMutex };
		m_assetChangedQueue.emplace_back(assetHandle, state);
	}

	void AssetManager::RemoveAssetFromRegistry(AssetHandle assetHandle)
	{
		bool assetExists = false;

		{
			ReadLock lock{ m_assetRegistryMutex };
			assetExists = m_assetRegistry.contains(assetHandle);
		}

		if (!assetExists)
		{
			VT_LOG(LogVerbosity::Warning, "[AssetManager] Trying to remove invalid asset {0} from registry!", assetHandle);
			return;
		}

		AssetMetadata metadata = s_nullMetadata;

		{
			ReadLock lock{ m_assetRegistryMutex };
			metadata = m_assetRegistry.at(assetHandle);
		}

		if (!metadata.IsValid())
		{
			VT_LOG(LogVerbosity::Warning, "[AssetManager] Trying to remove invalid asset {0} from registry!", assetHandle);
			return;
		}

		{
			WriteLock lock{ m_assetRegistryMutex };
			m_assetRegistry.erase(assetHandle);
		}

		{
			WriteLock lock{ m_assetCacheMutex };
			m_assetCache.erase(assetHandle);
		}

#ifdef VT_DEBUG
		const auto cleanFilePath = GetCleanAssetFilePath(metadata.filePath);
		VT_LOG(LogVerbosity::Info, "[AssetManager] Removed asset {0} with handle {1} from registry!", assetHandle, cleanFilePath);
#endif
	}

	void AssetManager::RemoveAssetFromRegistry(const std::filesystem::path& filePath)
	{
		AssetMetadata metadata = s_nullMetadata;

		{
			ReadLock lock{ m_assetRegistryMutex };
			metadata = GetMetadataFromFilePath(filePath);
		}

		if (!metadata.IsValid())
		{
			VT_LOG(LogVerbosity::Warning, "[AssetManager] Trying to remove invalid asset {0} from registry!", filePath);
			return;
		}

		{
			WriteLock lock{ m_assetRegistryMutex };
			m_assetRegistry.erase(metadata.handle);
		}

		{
			WriteLock lock{ m_assetCacheMutex };
			m_assetCache.erase(metadata.handle);
		}

#ifdef VT_DEBUG
		const auto cleanFilePath = GetCleanAssetFilePath(metadata.filePath);
		VT_LOG(LogVerbosity::Info, "[AssetManager] Removed asset {0} with handle {1} from registry!", metadata.handle, cleanFilePath);
#endif
	}

	void AssetManager::RemoveFullFolderFromRegistry(const std::filesystem::path& folderPath)
	{
		if (folderPath.empty())
		{
			VT_LOG(LogVerbosity::Warning, "[AssetManager] Trying to remove invalid directory {0}!", folderPath);
			return;
		}

		Vector<AssetHandle> filesToRemove{};
		{
			ReadLock lock{ m_assetRegistryMutex };
			const std::string sourceDirLower = ::Utility::ToLower(folderPath.string());

			for (const auto& [handle, metaData] : m_assetRegistry)
			{
				const std::string filePathLower = ::Utility::ToLower(metaData.filePath.string());

				if (auto it = filePathLower.find(sourceDirLower); it != std::string::npos)
				{
					filesToRemove.emplace_back(handle);
				}
			}
		}

		{

			for (const auto& handle : filesToRemove)
			{
				const auto metadata = GetMetadataFromHandle(handle);

				{
					WriteLock lock{ m_assetCacheMutex };
					if (m_assetCache.contains(handle))
					{
						m_assetCache.erase(handle);
					}
				}


				{
					WriteLock registryMutex{ m_assetRegistryMutex };
					if (m_assetRegistry.contains(handle))
					{
						m_assetRegistry.erase(handle);
					}
				}

#ifdef VT_DEBUG
				VT_LOG(LogVerbosity::Info, "[AssetManager] Removed asset with handle {0} from registry!", handle);
#endif
			}
		}
	}

	void AssetManager::AddDependencyToAsset(AssetHandle handle, AssetHandle dependency)
	{
		if (handle == Asset::Null() || dependency == Asset::Null())
		{
			return;
		}

		Get().m_dependencyGraph->AddDependencyToAsset(handle, dependency);
	}

	Vector<AssetHandle> AssetManager::GetAssetsDependentOn(AssetHandle handle)
	{
		if (handle == Asset::Null())
		{
			return {};
		}

		return Get().m_dependencyGraph->GetAssetsDependentOn(handle);
	}

	const AssetHandle AssetManager::AddAssetToRegistry(const std::filesystem::path& filePath, AssetHandle handle /*= 0*/)
	{
		{
			ReadLock lock{ m_assetRegistryMutex };
			const std::filesystem::path cleanPath = GetCleanAssetFilePath(filePath);
			const auto& metadata = GetMetadataFromFilePath(filePath);

			if (metadata.IsValid())
			{
				return metadata.handle;
			}
		}

		const auto newHandle = (handle != 0) ? handle : AssetHandle{};
		const auto cleanFilePath = GetCleanAssetFilePath(filePath);

		{
			WriteLock lock{ m_assetRegistryMutex };
			AssetMetadata& metadata = m_assetRegistry[newHandle];
			metadata.handle = newHandle;
			metadata.filePath = cleanFilePath;
			metadata.type = GetAssetTypeFromExtension(filePath.extension().string());
		}

		m_dependencyGraph->AddAssetToGraph(newHandle);
		return newHandle;
	}

	void AssetManager::AddAssetToRegistry(const std::filesystem::path& filePath, AssetHandle handle, AssetType type)
	{
		{
			ReadLock lock{ m_assetRegistryMutex };
			const std::filesystem::path cleanPath = GetCleanAssetFilePath(filePath);
			const auto& metadata = GetMetadataFromFilePath(filePath);

			if (metadata.IsValid())
			{
				return;
			}
		}

		const auto newHandle = handle;
		const auto cleanFilePath = GetCleanAssetFilePath(filePath);

		{
			WriteLock lock{ m_assetRegistryMutex };
			AssetMetadata& metadata = m_assetRegistry[newHandle];
			metadata.handle = newHandle;
			metadata.filePath = cleanFilePath;
			metadata.type = type;
		}

		m_dependencyGraph->AddAssetToGraph(newHandle);
	}

	bool AssetManager::IsLoaded(AssetHandle handle)
	{
		ReadLock lock{ Get().m_assetCacheMutex };
		return Get().m_assetCache.contains(handle);
	}

	bool AssetManager::IsEngineAsset(const std::filesystem::path& path)
	{
		const auto pathSplit = ::Utility::SplitStringsByCharacter(path.string(), '/');
		if (!pathSplit.empty())
		{
			std::string lowerFirstPart = ::Utility::ToLower(pathSplit.front());
			if (::Utility::StringContains(lowerFirstPart, "engine") || ::Utility::StringContains(lowerFirstPart, "editor"))
			{
				return true;
			}
		}

		return false;
	}

	Ref<Asset> AssetManager::GetAssetRaw(AssetHandle assetHandle)
	{
		if (assetHandle == Asset::Null())
		{
			return nullptr;
		}

		{
			ReadLock lock{ m_assetCacheMutex };
			auto it = m_assetCache.find(assetHandle);
			if (it != m_assetCache.end())
			{
				return it->second;
			}
		}

		const AssetType assetType = GetAssetTypeFromHandle(assetHandle);
		if (assetType == AssetType::None)
		{
			return nullptr;
		}

		Ref<Asset> asset = m_assetFactory->CreateAssetOfType(assetType);
		LoadAsset(assetHandle, asset);

		return asset;
	}

	Ref<Asset> AssetManager::QueueAssetRaw(AssetHandle assetHandle)
	{
		if (assetHandle == Asset::Null())
		{
			return nullptr;
		}

		const AssetType assetType = GetAssetTypeFromHandle(assetHandle);
		if (assetType == AssetType::None)
		{
			return nullptr;
		}

		Ref<Asset> asset = m_assetFactory->CreateAssetOfType(assetType);
		asset->SetFlag(AssetFlag::Queued, true);
		Get().QueueAssetInternal(assetHandle, asset);

		return asset;
	}

	void AssetManager::Update()
	{
		Get().UpdateInternal();
	}

	AssetType AssetManager::GetAssetTypeFromHandle(const AssetHandle& handle)
	{
		ReadLock lock{ Get().m_assetRegistryMutex };

		if (Get().m_assetRegistry.contains(handle))
		{
			return Get().m_assetRegistry.at(handle).type;
		}

		return AssetType::None;
	}

	AssetType AssetManager::GetAssetTypeFromPath(const std::filesystem::path& path)
	{
		return GetAssetTypeFromHandle(GetAssetHandleFromFilePath(path));
	}

	AssetType AssetManager::GetAssetTypeFromExtension(const std::string& extension)
	{
		std::string ext = ::Utility::ToLower(extension);
		if (!s_assetExtensionsMap.contains(ext))
		{
			return AssetType::None;
		}

		return s_assetExtensionsMap.at(ext);
	}

	AssetHandle AssetManager::GetAssetHandleFromFilePath(const std::filesystem::path& filePath)
	{
		const auto& metadata = GetMetadataFromFilePath(filePath);
		if (!metadata.IsValid())
		{
			return 0;
		}

		return metadata.handle;
	}

	const AssetMetadata& AssetManager::GetMetadataFromHandle(AssetHandle handle)
	{
		auto& instance = Get();
		ReadLock lock{ instance.m_assetRegistryMutex };

		if (!instance.m_assetRegistry.contains(handle))
		{
			return s_nullMetadata;
		}

		return instance.m_assetRegistry.at(handle);
	}

	const AssetMetadata& AssetManager::GetMetadataFromFilePath(const std::filesystem::path filePath)
	{
		auto& instance = Get();
		ReadLock lock{ instance.m_assetRegistryMutex };

		std::filesystem::path cleanPath = GetRelativePath(filePath);

		for (const auto& [handle, metaData] : instance.m_assetRegistry)
		{
			if (metaData.filePath == cleanPath)
			{
				return metaData;
			}
		}

		return s_nullMetadata;
	}

	const std::unordered_map<AssetHandle, AssetMetadata>& AssetManager::GetAssetRegistry()
	{
		return Get().m_assetRegistry;
	}

	std::unordered_map<AssetHandle, AssetMetadata>& AssetManager::GetAssetRegistryMutable()
	{
		return Get().m_assetRegistry;
	}

	const std::filesystem::path AssetManager::GetFilePathFromAssetHandle(AssetHandle handle)
	{
		const auto& metadata = GetMetadataFromHandle(handle);
		if (!metadata.IsValid())
		{
			return {};
		}

		return metadata.filePath;
	}

	const std::filesystem::path AssetManager::GetContextPath(const std::filesystem::path& path)
	{
		std::filesystem::path projDir;

		if (!IsEngineAsset(path))
		{
			projDir = ProjectManager::GetDirectory();
		}

		return projDir;
	}

	std::string AssetManager::GetExtensionFromAssetType(AssetType type)
	{
		for (const auto& [ext, asset] : s_assetExtensionsMap)
		{
			if (asset == type)
			{
				return ext;
			}
		}
		return "Null";
	}

	const std::filesystem::path AssetManager::GetFilePathFromFilename(const std::string& filename)
	{
		auto& instance = Get();

		ReadLock lock{ instance.m_assetRegistryMutex };

		for (const auto& [handle, metadata] : instance.m_assetRegistry)
		{
			if (metadata.filePath.filename() == filename)
			{
				return metadata.filePath;
			}
		}

		return {};
	}

	bool AssetManager::IsSourceAsset(AssetHandle handle)
	{
		auto& instance = Get();
		ReadLock lock{ instance.m_assetRegistryMutex };

		const AssetType type = GetAssetTypeFromHandle(handle);
		return IsSourceAsset(type);
	}

	bool AssetManager::IsSourceAsset(AssetType type)
	{
		switch (type)
		{
			case AssetType::MeshSource:
			case AssetType::ShaderSource:
			case AssetType::TextureSource:
				return true;
		}

		return false;
	}

	bool AssetManager::ExistsInRegistry(AssetHandle handle)
	{
		if (Get().m_memoryAssets.contains(handle))
		{
			return true;
		}

		return Get().m_assetRegistry.contains(handle);
	}

	bool AssetManager::ExistsInRegistry(const std::filesystem::path& filePath)
	{
		const auto& metadata = GetMetadataFromFilePath(filePath);
		return metadata.IsValid();
	}

	const std::filesystem::path AssetManager::GetFilesystemPath(AssetHandle handle)
	{
		const auto path = GetFilePathFromAssetHandle(handle);
		return GetContextPath(path) / path;
	}

	const std::filesystem::path AssetManager::GetFilesystemPath(const std::filesystem::path& filePath)
	{
		return GetContextPath(filePath) / filePath;
	}

	const std::filesystem::path AssetManager::GetRelativePath(const std::filesystem::path& path)
	{
		std::filesystem::path relativePath = path.lexically_normal();
		std::string temp = path.string();

		if (temp.find(ProjectManager::GetDirectory().string()) != std::string::npos)
		{
			relativePath = std::filesystem::relative(path, ProjectManager::GetDirectory());
		}
		else if (temp.find(ProjectManager::GetEngineDirectory().string()) != std::string::npos)
		{
			relativePath = std::filesystem::relative(path, ProjectManager::GetEngineDirectory());
		}

		if (relativePath.empty())
		{
			relativePath = path.lexically_normal();
		}

		return GetCleanAssetFilePath(relativePath);
	}

	void AssetManager::QueueAssetInternal(AssetHandle assetHandle, Ref<Asset>& asset)
	{
		// Check if asset is loaded
		{
			ReadLock lock{ m_assetCacheMutex };

			if (m_assetCache.contains(assetHandle))
			{
				asset = m_assetCache.at(assetHandle);
				return;
			}
		}

		AssetMetadata metadata = s_nullMetadata;

		{
			ReadLock registryLock{ m_assetRegistryMutex };
			metadata = GetMetadataFromHandle(assetHandle);
		}

		if (!metadata.IsValid())
		{
			asset->SetFlag(AssetFlag::Invalid, true);
			return;
		}

		{
			WriteLock lock{ m_assetCacheMutex };
			asset->handle = metadata.handle;
			m_assetCache.emplace(assetHandle, asset);
		}

		if (!m_assetSerializers.contains(metadata.type))
		{
			VT_LOG(LogVerbosity::Error, "[AssetManager]: No importer for asset found!");
			asset->SetFlag(AssetFlag::Invalid, true);
			return;
		}

		m_dependencyGraph->AddAssetToGraph(assetHandle);

		// If not, queue
		{
			auto& threadPool = Application::GetThreadPool();

			threadPool.SubmitTask([this, metadata](AssetHandle handle)
			{
				Ref<Asset> asset;
				{
					ReadLock lock{ m_assetCacheMutex };
					asset = m_assetCache.at(handle);
				}

				if (handle != Asset::Null())
				{
					asset->handle = handle;
				}

				asset->assetName = metadata.filePath.stem().string();

				{
#ifndef VT_DIST
					ScopedTimer timer{};
#endif

					m_assetSerializers.at(metadata.type)->Deserialize(metadata, asset);

#ifndef VT_DIST
					VT_LOG(LogVerbosity::Trace, "[AssetManager]: Loaded asset {0} with handle {1} in {2} seconds!", metadata.filePath.string().c_str(), asset->handle, timer.GetTime<Time::Seconds>());
#endif
				}

				asset->SetFlag(AssetFlag::Queued, false);

				{
					ReadLock lock{ m_assetRegistryMutex };
					m_assetRegistry.at(handle).isLoaded = true;
				}

				{
					WriteLock lock{ m_assetCacheMutex };
					m_assetCache[handle] = asset;
				}

				m_dependencyGraph->OnAssetChanged(handle, AssetChangedState::Updated);
				QueueAssetChanged(asset->handle, AssetChangedState::Updated);

			}, assetHandle);

#ifndef VT_DIST
			VT_LOG(LogVerbosity::Trace, "[AssetManager]: Queued asset {0} for loading!", metadata.filePath);
#endif
		}
	}

	AssetMetadata& AssetManager::GetMetadataFromHandleMutable(AssetHandle handle)
	{
		auto& instance = Get();

		if (!instance.m_assetRegistry.contains(handle))
		{
			return s_nullMetadata;
		}

		return instance.m_assetRegistry.at(handle);
	}

	AssetMetadata& AssetManager::GetMetadataFromFilePathMutable(const std::filesystem::path filePath)
	{
		auto& instance = Get();

		for (auto& [handle, metaData] : instance.m_assetRegistry)
		{
			if (metaData.filePath == filePath)
			{
				return metaData;
			}
		}

		return s_nullMetadata;
	}

	const std::filesystem::path AssetManager::GetCleanAssetFilePath(const std::filesystem::path& filePath)
	{
		auto pathClean = ::Utility::ReplaceCharacter(filePath.string(), '\\', '/');
		return pathClean;
	}

	Vector<std::filesystem::path> AssetManager::GetEngineAssetFiles()
	{
		Vector<std::filesystem::path> files;
		const std::string ext(".vtasset");

		// Engine Directory
		for (auto& p : std::filesystem::recursive_directory_iterator(ProjectManager::GetEngineDirectory() / "Engine"))
		{
			if (p.path().extension() == ext)
			{
				files.emplace_back(GetRelativePath(p.path()));
			}
		}

		const auto editorFolder = ProjectManager::GetEngineDirectory() / "Editor";
		if (FileSystem::Exists(editorFolder))
		{
			for (auto& p : std::filesystem::recursive_directory_iterator(editorFolder))
			{
				if (p.path().extension() == ext)
				{
					files.emplace_back(GetRelativePath(p.path()));
				}
			}
		}

		return files;
	}

	Vector<std::filesystem::path> AssetManager::GetProjectAssetFiles()
	{
		Vector<std::filesystem::path> files;
		std::string ext(".vtasset");

		// Project Directory
		const auto assetsDir = ProjectManager::GetProject().projectDirectory / ProjectManager::GetProject().assetsDirectory;

		if (FileSystem::Exists(assetsDir))
		{
			for (auto& p : std::filesystem::recursive_directory_iterator(assetsDir))
			{
				if (p.path().extension() == ext)
				{
					files.emplace_back(GetRelativePath(p.path()));
				}
			}
		}

		return files;
	}
}
