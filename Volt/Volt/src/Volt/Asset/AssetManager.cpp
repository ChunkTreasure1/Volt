#include "vtpch.h"
#include "AssetManager.h"

#include "Volt/Core/Base.h"
#include "Volt/Core/Application.h"

#include "Volt/Asset/Importers/AssetImporter.h"

#include "Volt/Asset/Importers/MeshTypeImporter.h"
#include "Volt/Asset/Importers/TextureImporter.h"
#include "Volt/Asset/Importers/MeshSourceImporter.h"
#include "Volt/Asset/Importers/VTNavMeshImporter.h"
#include "Volt/Asset/Importers/SkeletonImporter.h"
#include "Volt/Asset/Importers/AnimationImporter.h"
#include "Volt/Asset/Importers/SceneImporter.h"
#include "Volt/Asset/Importers/AnimatedCharacterImporter.h"
#include "Volt/Asset/Importers/AnimationGraphImporter.h"
#include "Volt/Asset/Importers/PrefabImporter.h"
#include "Volt/Asset/Importers/BehaviorTreeImporter.h"
#include "Volt/Asset/Importers/NetContractImporter.h"
#include "Volt/Asset/Importers/ParticlePresetImporter.h"

#include "Volt/Platform/ThreadUtility.h"

#include "Volt/Utility/FileSystem.h"
#include "Volt/Utility/YAMLSerializationHelpers.h"
#include "Volt/Utility/SerializationMacros.h"

namespace Volt
{
	AssetManager::AssetManager()
	{
		VT_CORE_ASSERT(!s_instance, "AssetManager already exists!");
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

		m_assetImporters.emplace(AssetType::MeshSource, CreateScope<MeshSourceImporter>());
		m_assetImporters.emplace(AssetType::Texture, CreateScope<TextureSourceImporter>());
		m_assetImporters.emplace(AssetType::Shader, CreateScope<ShaderImporter>());
		m_assetImporters.emplace(AssetType::Material, CreateScope<MaterialImporter>());
		m_assetImporters.emplace(AssetType::Mesh, CreateScope<MeshSourceImporter>());
		m_assetImporters.emplace(AssetType::NavMesh, CreateScope<VTNavMeshImporter>());
		m_assetImporters.emplace(AssetType::Scene, CreateScope<SceneImporter>());
		m_assetImporters.emplace(AssetType::Skeleton, CreateScope<SkeletonImporter>());
		m_assetImporters.emplace(AssetType::AnimationGraph, CreateScope<AnimationGraphImporter>());
		m_assetImporters.emplace(AssetType::Animation, CreateScope<AnimationImporter>());
		m_assetImporters.emplace(AssetType::AnimatedCharacter, CreateScope<AnimatedCharacterImporter>());
		m_assetImporters.emplace(AssetType::ParticlePreset, CreateScope<ParticlePresetImporter>());
		m_assetImporters.emplace(AssetType::Prefab, CreateScope<PrefabImporter>());
		m_assetImporters.emplace(AssetType::Font, CreateScope<FontImporter>());
		m_assetImporters.emplace(AssetType::PhysicsMaterial, CreateScope<PhysicsMaterialImporter>());
		m_assetImporters.emplace(AssetType::Video, CreateScope<VideoImporter>());
		m_assetImporters.emplace(AssetType::BehaviorGraph, CreateScope<BehaviorTreeImporter>());
		m_assetImporters.emplace(AssetType::BlendSpace, CreateScope<BlendSpaceImporter>());
		m_assetImporters.emplace(AssetType::PostProcessingStack, CreateScope<PostProcessingStackImporter>());
		m_assetImporters.emplace(AssetType::PostProcessingMaterial, CreateScope<PostProcessingMaterialImporter>());
		m_assetImporters.emplace(AssetType::NetContract, CreateScope<NetContractImporter>());

		LoadAssetMetaFiles();
	}

	void AssetManager::Shutdown()
	{
		TextureImporter::Shutdown();
		MeshTypeImporter::Shutdown();
	}

	void AssetManager::AddDependency(AssetHandle asset, const std::filesystem::path& dependency)
	{
		auto& metaData = GetMetadataFromHandleMutable(asset);
		const auto& dependencyMetaData = GetMetadataFromFilePath(dependency);

		AddDependency(asset, dependencyMetaData.handle);
	}

	void AssetManager::AddDependency(AssetHandle asset, AssetHandle dependency)
	{
		auto& metadata = GetMetadataFromHandleMutable(asset);
		const auto& dependencyMetaData = GetMetadataFromHandle(dependency);

		if (!metadata.IsValid())
		{
			VT_CORE_WARN("[AssetManager] Trying to add dependency {0} to invalid asset {1}", dependency, asset);
			return;
		}

		if (!dependencyMetaData.IsValid())
		{
			VT_CORE_WARN("[AssetManager] Trying to add invalid dependency {0} to asset {1}", dependency, asset);
			return;
		}

		auto it = std::find_if(metadata.dependencies.begin(), metadata.dependencies.end(), [&](AssetHandle dep)
		{
			return dep == dependencyMetaData.handle;
		});

		if (it != metadata.dependencies.end())
		{
			return;
		}

		metadata.dependencies.emplace_back(dependencyMetaData.handle);
		SerializeAssetMetaFile(metadata.handle);
	}

	const std::vector<AssetHandle> AssetManager::GetAllAssetsOfType(AssetType wantedAssetType)
	{
		auto& instance = Get();

		ReadLock lock{ instance.m_assetRegistryMutex };
		std::vector<AssetHandle> result;

		for (const auto& [handle, metadata] : instance.m_assetRegistry)
		{
			if (metadata.type == wantedAssetType || wantedAssetType == AssetType::None)
			{
				result.emplace_back(handle);
			}
		}

		return result;
	}

	const std::vector<AssetHandle> AssetManager::GetAllAssetsWithDependency(const std::filesystem::path& dependencyFilePath)
	{
		const std::string pathString = Utils::ReplaceCharacter(Get().GetRelativePath(dependencyFilePath).string(), '\\', '/');
		std::vector<AssetHandle> result{};

		auto& instance = Get();
		ReadLock lock{ instance.m_assetRegistryMutex };

		for (const auto& [handle, metadata] : instance.m_assetRegistry)
		{
			for (const auto& dependency : metadata.dependencies)
			{
				const auto& depMeta = GetMetadataFromHandle(dependency);
				if (!depMeta.IsValid())
				{
					continue;
				}

				if (depMeta.filePath == dependencyFilePath)
				{
					result.emplace_back(handle);
					break;
				}
			}
		}

		return result;
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

		{ 
			ReadLock lock{ m_assetRegistryMutex };

			AssetMetadata& metadata = GetMetadataFromHandleMutable(assetHandle);
			if (!metadata.IsValid())
			{
				return;
			}

			if (!m_assetImporters.contains(metadata.type))
			{
				VT_CORE_WARN("[AssetManager] No importer for asset found!");
				return;
			}

			m_assetImporters.at(metadata.type)->Load(metadata, asset);
			if (!asset) { return; }

#ifdef VT_DEBUG
			VT_CORE_TRACE("[AssetManager] Loaded asset {0} with handle {1}!", metadata.filePath, assetHandle);
#endif	

			asset->handle = metadata.handle;
			asset->name = metadata.filePath.stem().string();

			metadata.isLoaded = true;
		}

		{
			WriteLock lock{ m_assetCacheMutex };
			m_assetCache.emplace(asset->handle, asset);
		}
	}

	void AssetManager::LoadAssetMetaFiles()
	{
		for (auto file : GetMetaFiles())
		{
			DeserializeAssetMetaFile(file);
		}
	}

	void AssetManager::Unload(AssetHandle assetHandle)
	{
		WriteLock lock{ m_assetCacheMutex };

		auto it = m_assetCache.find(assetHandle);
		if (it == m_assetCache.end())
		{
			VT_CORE_WARN("[AssetManager] Unable to unload asset with handle {0}, it has not been loaded!", assetHandle);
			return;
		}

		m_assetCache.erase(it);
	}

	void AssetManager::ReloadAsset(const std::filesystem::path& path)
	{
		AssetHandle handle = GetAssetHandleFromFilePath(path);
		if (handle == Asset::Null())
		{
			VT_CORE_ERROR("[AssetManager] Asset with path {0} is not loaded!", path.string());
			return;
		}

		ReloadAsset(handle);
	}

	void AssetManager::ReloadAsset(AssetHandle handle)
	{
		Unload(handle);

		Ref<Asset> asset;
		LoadAsset(handle, asset);
	}

	void AssetManager::SaveAssetAs(Ref<Asset> asset, const std::filesystem::path& targetFilePath)
	{
		auto& instance = Get();

		if (instance.m_assetImporters.find(asset->GetType()) == instance.m_assetImporters.end())
		{
			VT_CORE_ERROR("[AssetManager] No exporter for asset {0} found!", asset->handle);
			return;
		}

		if (!asset->IsValid())
		{
			VT_CORE_ERROR("[AssetManager] Unable to save invalid asset {0}!", asset->handle);
			return;
		}

		if (!instance.m_assetRegistry.contains(asset->handle))
		{
			WriteLock lock{ instance.m_assetRegistryMutex };

			AssetMetadata& metaData = instance.m_assetRegistry[asset->handle];
			metaData.filePath = GetCleanAssetFilePath(targetFilePath);
			metaData.handle = asset->handle;
			metaData.isLoaded = true;
			metaData.type = asset->GetType();
		}

		{
			WriteLock lock{ instance.m_assetCacheMutex };
			if (!instance.m_assetCache.contains(asset->handle))
			{
				instance.m_assetCache.emplace(asset->handle, asset);
			}
		}

		instance.SerializeAssetMetaFile(asset->handle);
	}

	void AssetManager::SaveAsset(const Ref<Asset> asset)
	{
		auto& instance = Get();

		if (instance.m_assetImporters.find(asset->GetType()) == instance.m_assetImporters.end())
		{
			VT_CORE_ERROR("[AssetManager] No exporter for asset {0} found!", asset->handle);
			return;
		}

		if (!asset->IsValid())
		{
			VT_CORE_ERROR("[AssetManager] Unable to save invalid asset {0}!", asset->handle);
			return;
		}

		AssetMetadata metadata = s_nullMetadata;

		{
			ReadLock lock{ instance.m_assetRegistryMutex };
			metadata = GetMetadataFromHandle(asset->handle);
		}

		instance.m_assetImporters[metadata.type]->Save(metadata, asset);

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
				VT_CORE_WARN("[AssetMananger] Unable to move invalid asset {0}!", asset->handle);
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

		RemoveMetaFile(assetFilePath);
		SerializeAssetMetaFile(asset->handle);
	}

	void AssetManager::MoveAsset(AssetHandle assetHandle, const std::filesystem::path& targetDir)
	{
		std::filesystem::path assetFilePath;

		{
			ReadLock lock{ m_assetRegistryMutex };

			const auto& assetMetaData = GetMetadataFromHandle(assetHandle);
			if (!assetMetaData.IsValid())
			{
				VT_CORE_WARN("[AssetMananger] Unable to move invalid asset {0}!", assetHandle);
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

		RemoveMetaFile(assetFilePath);
		SerializeAssetMetaFile(assetHandle);
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

		RemoveMetaFile(sourcePath);
		SerializeAssetMetaFile(assetHandle);
	}

	void AssetManager::MoveFullFolder(const std::filesystem::path& sourceDir, const std::filesystem::path& targetDir)
	{
		if (sourceDir.empty())
		{
			VT_CORE_WARN("[AssetManager] Trying to move invalid directory!");
			return;
		}

		if (targetDir.empty())
		{
			VT_CORE_WARN("[AssetManager] Trying to move directory {0} to an invalid directory!");
			return;
		}

		std::vector<AssetHandle> filesToMove{};
		{
			ReadLock lock{ m_assetRegistryMutex };
			const std::string sourceDirLower = Utils::ToLower(sourceDir.string());

			for (const auto& [handle, metaData] : m_assetRegistry)
			{
				const std::string filePathLower = Utils::ToLower(metaData.filePath.string());

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
				const size_t directoryStringLoc = Utils::ToLower(newPath).find(Utils::ToLower(sourceDir.string()));

				if (directoryStringLoc == std::string::npos)
				{
					continue;
				}

				newPath.erase(directoryStringLoc, sourceDir.string().length());
				newPath.insert(directoryStringLoc, targetDir.string());

				RemoveMetaFile(metadata.filePath);
				metadata.filePath = GetCleanAssetFilePath(newPath);

				SerializeAssetMetaFile(metadata.handle);
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
				VT_CORE_WARN("[AssetManager] Trying to rename invalid asset!");
				return;
			}

			m_assetRegistry.at(assetHandle).filePath = newPath;
		}

		FileSystem::Rename(projDir / oldPath, newName);

		RemoveMetaFile(oldPath);
		SerializeAssetMetaFile(assetHandle);
	}

	void AssetManager::RenameAssetFolder(AssetHandle assetHandle, const std::filesystem::path& targetFilePath)
	{
		WriteLock lock{ m_assetRegistryMutex };

		auto& metadata = GetMetadataFromHandleMutable(assetHandle);
		if (!metadata.IsValid())
		{
			VT_CORE_WARN("[AssetManager] Trying to rename invalid asset {0}!", assetHandle);
			return;
		}

		RemoveMetaFile(metadata.filePath);
		metadata.filePath = GetCleanAssetFilePath(targetFilePath);
		SerializeAssetMetaFile(metadata.handle);
	}

	void AssetManager::RemoveAsset(AssetHandle assetHandle)
	{
		WriteLock lock{ m_assetRegistryMutex };

		const auto metadata = GetMetadataFromHandle(assetHandle);
		if (!metadata.IsValid())
		{
			VT_CORE_WARN("[AssetManager] Trying to remove invalid asset {0}!", assetHandle);
			return;
		}

		m_assetRegistry.erase(assetHandle);

		const std::filesystem::path filePath = metadata.filePath;
		const auto projDir = GetContextPath(filePath);

		{
			WriteLock lock{ m_assetCacheMutex };
			m_assetCache.erase(assetHandle);
		}

		FileSystem::MoveToRecycleBin(projDir / filePath);
		RemoveMetaFile(filePath);

#ifdef VT_DEBUG
		VT_CORE_INFO("[AssetManager] Removed asset {0} with handle {1}!", assetHandle, filePath.string());
#endif
	}

	void AssetManager::RemoveAsset(const std::filesystem::path& path)
	{
		WriteLock lock{ m_assetRegistryMutex };

		const auto metadata = GetMetadataFromFilePath(path);
		if (!metadata.IsValid())
		{
			VT_CORE_WARN("[AssetManager] Trying to remove invalid asset {0}!", path);
			return;
		}

		m_assetRegistry.erase(metadata.handle);

		const std::filesystem::path filePath = metadata.filePath;
		const auto projDir = GetContextPath(filePath);

		{
			WriteLock lock{ m_assetCacheMutex };
			m_assetCache.erase(metadata.handle);
		}

		FileSystem::MoveToRecycleBin(projDir / filePath);
		RemoveMetaFile(filePath);

#ifdef VT_DEBUG
		VT_CORE_INFO("[AssetManager] Removed asset {0} with handle {1}!", metadata.handle, filePath.string());
#endif
	}

	void AssetManager::RemoveMetaFile(const std::filesystem::path& filePath)
	{
		auto metafile = GetContextPath(filePath) / filePath;
		metafile.replace_filename(metafile.filename().string() + ".vtmeta");
		if (std::filesystem::exists(metafile))
		{
			std::filesystem::remove(metafile);
		}
	}

	void AssetManager::RemoveFromRegistry(AssetHandle assetHandle)
	{
		bool assetExists = false;

		{
			ReadLock lock{ m_assetRegistryMutex };
			assetExists = m_assetRegistry.contains(assetHandle);
		}

		if (!assetExists)
		{
			VT_CORE_WARN("[AssetManager] Trying to remove invalid asset {0} from registry!", assetHandle);
			return;
		}

		AssetMetadata metadata = s_nullMetadata;

		{
			ReadLock lock{ m_assetRegistryMutex };
			metadata = m_assetRegistry.at(assetHandle);
		}

		if (!metadata.IsValid())
		{
			VT_CORE_WARN("[AssetManager] Trying to remove invalid asset {0} from registry!", assetHandle);
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

		const auto cleanFilePath = GetCleanAssetFilePath(metadata.filePath);
		RemoveMetaFile(cleanFilePath);

#ifdef VT_DEBUG
		VT_CORE_INFO("[AssetManager] Removed asset {0} with handle {1} from registry!", assetHandle, cleanFilePath);
#endif
	}

	void AssetManager::RemoveFromRegistry(const std::filesystem::path& filePath)
	{
		AssetMetadata metadata = s_nullMetadata;

		{
			ReadLock lock{ m_assetRegistryMutex };
			metadata = GetMetadataFromFilePath(filePath);
		}

		if (!metadata.IsValid())
		{
			VT_CORE_WARN("[AssetManager] Trying to remove invalid asset {0} from registry!", filePath);
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

		const auto cleanFilePath = GetCleanAssetFilePath(metadata.filePath);
		RemoveMetaFile(cleanFilePath);

#ifdef VT_DEBUG
		VT_CORE_INFO("[AssetManager] Removed asset {0} with handle {1} from registry!", metadata.handle, cleanFilePath);
#endif
	}

	void AssetManager::RemoveFullFolderFromRegistry(const std::filesystem::path& folderPath)
	{
		if (folderPath.empty())
		{
			VT_CORE_WARN("[AssetManager] Trying to remove invalid directory {0}!", folderPath);
			return;
		}

		std::vector<AssetHandle> filesToRemove{};
		{
			ReadLock lock{ m_assetRegistryMutex };
			const std::string sourceDirLower = Utils::ToLower(folderPath.string());

			for (const auto& [handle, metaData] : m_assetRegistry)
			{
				const std::string filePathLower = Utils::ToLower(metaData.filePath.string());

				if (auto it = filePathLower.find(sourceDirLower); it != std::string::npos)
				{
					filesToRemove.emplace_back(handle);
				}
			}
		}

		{
			WriteLock lock{ m_assetCacheMutex };
			WriteLock registryMutex{ m_assetRegistryMutex };

			for (const auto& handle : filesToRemove)
			{
				const auto metadata = GetMetadataFromHandle(handle);

				if (m_assetCache.contains(handle))
				{
					m_assetCache.erase(handle);
				}

				if (m_assetRegistry.contains(handle))
				{
					m_assetRegistry.erase(handle);
				}

				RemoveMetaFile(metadata.filePath);

#ifdef VT_DEBUG
				VT_CORE_INFO("[AssetManager] Removed asset with handle {0} from registry!", handle);
#endif
			}
		}
	}

	const AssetHandle AssetManager::AddAssetToRegistry(const std::filesystem::path& filePath, AssetHandle handle /*= 0*/)
	{
		{
			ReadLock lock{ m_assetRegistryMutex };
			if (m_assetRegistry.contains(handle))
			{
				return handle;
			}
		}

		const auto newHandle = (handle != 0) ? handle : AssetHandle{};
		const auto cleanFilePath = GetCleanAssetFilePath(filePath);

		{
			WriteLock lock{ m_assetRegistryMutex };
			AssetMetadata& metadata = m_assetRegistry[newHandle];
			metadata.handle = newHandle;
			metadata.filePath = cleanFilePath;
			metadata.type = GetAssetTypeFromPath(filePath);
		}

		if (!HasAssetMetaFile(newHandle))
		{
			SerializeAssetMetaFile(newHandle);
		}

		return newHandle;
	}

	bool AssetManager::IsLoaded(AssetHandle handle)
	{
		ReadLock lock{ Get().m_assetCacheMutex };
		return Get().m_assetCache.contains(handle);
	}

	bool AssetManager::IsEngineAsset(const std::filesystem::path& path)
	{
		const auto pathSplit = Utils::SplitStringsByCharacter(path.string(), '/');
		if (!pathSplit.empty())
		{
			std::string lowerFirstPart = Utils::ToLower(pathSplit.front());
			if (lowerFirstPart.contains("engine") || lowerFirstPart.contains("editor"))
			{
				return true;
			}
		}

		return false;
	}

	Ref<Asset> AssetManager::GetAssetRaw(AssetHandle assetHandle)
	{
		{
			ReadLock lock{ m_assetCacheMutex };
			auto it = m_assetCache.find(assetHandle);
			if (it != m_assetCache.end())
			{
				return it->second;
			}
		}

		Ref<Asset> asset;
		LoadAsset(assetHandle, asset);

		return asset;
	}

	Ref<Asset> AssetManager::QueueAssetRaw(AssetHandle assetHandle)
	{
		Ref<Asset> asset = CreateRef<Asset>();
		asset->SetFlag(AssetFlag::Queued, true);
		Get().QueueAssetInternal(assetHandle, asset);

		return asset;
	}

	AssetType AssetManager::GetAssetTypeFromHandle(const AssetHandle& handle)
	{
		return GetAssetTypeFromExtension(GetFilePathFromAssetHandle(handle).extension().string());
	}

	AssetType AssetManager::GetAssetTypeFromPath(const std::filesystem::path& path)
	{
		return GetAssetTypeFromExtension(path.extension().string());
	}

	AssetType AssetManager::GetAssetTypeFromExtension(const std::string& extension)
	{
		std::string ext = Utils::ToLower(extension);
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

		if (!instance.m_assetRegistry.contains(handle))
		{
			return s_nullMetadata;
		}

		return instance.m_assetRegistry.at(handle);
	}

	const AssetMetadata& AssetManager::GetMetadataFromFilePath(const std::filesystem::path filePath)
	{
		auto& instance = Get();

		for (const auto& [handle, metaData] : instance.m_assetRegistry)
		{
			if (metaData.filePath == filePath)
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

	bool AssetManager::IsSourceFile(AssetHandle handle) const
	{
		const AssetType type = GetAssetTypeFromHandle(handle);
		switch (type)
		{
			case AssetType::MeshSource:
			case AssetType::ShaderSource:
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
			if (relativePath.empty())
			{
				relativePath = path.lexically_normal();
			}
		}

		return relativePath;
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
			VT_CORE_ERROR("[AssetManager] Trying to queue invalid asset {0}!", assetHandle);
			return;
		}

		{
			WriteLock lock{ m_assetCacheMutex };
			asset->handle = metadata.handle;
			m_assetCache.emplace(assetHandle, asset);
		}

		// If not, queue
		{
			auto& threadPool = Application::GetThreadPool();

			threadPool.SubmitTask([this, metadata](AssetHandle handle)
			{
				if (!m_assetImporters.contains(metadata.type))
				{
					VT_CORE_ERROR("No importer for asset found!");
					return;
				}

				Ref<Asset> asset;
				{
					ReadLock lock{ m_assetCacheMutex };
					asset = m_assetCache.at(handle);
				}

				m_assetImporters.at(metadata.type)->Load(metadata, asset);
				if (handle != Asset::Null())
				{
					asset->handle = handle;
				}

				asset->name = metadata.filePath.stem().string();

#ifndef VT_DIST
				VT_CORE_INFO("[AssetManager] Loaded asset {0} with handle {1}!", metadata.filePath.string().c_str(), asset->handle);
#endif

				asset->SetFlag(AssetFlag::Queued, false);

				{
					ReadLock lock{ m_assetRegistryMutex };
					m_assetRegistry.at(handle).isLoaded = true;
				}

				{
					WriteLock lock{ m_assetCacheMutex };
					m_assetCache[handle] = asset;
				}

			}, assetHandle);

#ifndef VT_DIST
			VT_CORE_TRACE("[AssetManager] Queued asset {0} for loading!", metadata.filePath.string());
#endif
		}
	}

	void AssetManager::QueueAssetInternal(AssetHandle assetHandle, Ref<Asset>& asset, const std::function<void()>& loadedCallback)
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
			VT_CORE_ERROR("[AssetManager] Trying to queue invalid asset {0}!", assetHandle);
			return;
		}

		{
			WriteLock lock{ m_assetCacheMutex };
			asset->handle = metadata.handle;
			m_assetCache.emplace(assetHandle, asset);
		}

		// If not, queue
		{
			auto& threadPool = Application::GetThreadPool();

			threadPool.SubmitTask([this, metadata, loadedCallback](AssetHandle handle)
			{
				if (!metadata.IsValid())
				{
					VT_CORE_ERROR("[AssetManager] Trying to load asset {0} which is invalid!", handle);
					return;
				}

				if (!m_assetImporters.contains(metadata.type))
				{
					VT_CORE_ERROR("No importer for asset found!");
					return;
				}

				Ref<Asset> asset;
				{
					ReadLock lock{ m_assetCacheMutex };
					asset = m_assetCache.at(handle);
				}

				m_assetImporters.at(metadata.type)->Load(metadata, asset);
				if (handle != Asset::Null())
				{
					asset->handle = handle;
				}

				asset->name = metadata.filePath.stem().string();

#ifndef VT_DIST
				VT_CORE_INFO("[AssetManager] Loaded asset {0} with handle {1}!", metadata.filePath.string().c_str(), asset->handle);
#endif

				asset->SetFlag(AssetFlag::Queued, false);

				{
					ReadLock lock{ m_assetRegistryMutex };
					m_assetRegistry.at(handle).isLoaded = true;
				}

				{
					WriteLock lock{ m_assetCacheMutex };
					m_assetCache[handle] = asset;
				}

				loadedCallback();

			}, assetHandle);
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
		auto pathClean = Utils::ReplaceCharacter(filePath.string(), '\\', '/');
		return pathClean;
	}

	void AssetManager::SerializeAssetMetaFile(AssetHandle assetHandle)
	{
		const auto& metadata = GetMetadataFromHandle(assetHandle);
		if (!metadata.IsValid())
		{
			VT_CORE_WARN("[AssetManager] Unable to save meta file for invalid asset {0}!", metadata.filePath);
			return;
		}

		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Metadata" << YAML::Value;
		{
			out << YAML::BeginMap;
			VT_SERIALIZE_PROPERTY(assetHandle, metadata.handle, out);
			VT_SERIALIZE_PROPERTY(filePath, metadata.filePath, out);
			VT_SERIALIZE_PROPERTY(type, (uint32_t)metadata.type, out);

			out << YAML::Key << "Dependencies" << YAML::Value << metadata.dependencies;
			out << YAML::EndMap;
		}
		out << YAML::EndMap;
		out << YAML::EndMap;

		auto metaPath = GetFilesystemPath(assetHandle);
		metaPath.replace_filename(metaPath.filename().string() + ".vtmeta");

		std::ofstream fout(metaPath);
		fout << out.c_str();
		fout.close();
	}

	bool AssetManager::HasAssetMetaFile(AssetHandle assetHandle)
	{
		auto metaPath = GetFilesystemPath(assetHandle);
		metaPath.replace_filename(metaPath.filename().string() + ".vtmeta");

		return FileSystem::Exists(metaPath);
	}

	void AssetManager::DeserializeAssetMetaFile(std::filesystem::path metaFilePath)
	{
		if (!std::filesystem::exists(metaFilePath))
		{
			return;
		}

		std::ifstream file(metaFilePath);
		if (!file.is_open())
		{
			VT_CORE_CRITICAL("[AssetManager] Failed to open asset registry file: {0}!", metaFilePath.string().c_str());
			return;
		}

		std::stringstream strStream;
		strStream << file.rdbuf();
		file.close();

		YAML::Node root;
		try
		{
			root = YAML::Load(strStream.str());
		}
		catch (std::exception& e)
		{
			VT_CORE_CRITICAL("[AssetManager] Meta file {0} contains invalid YAML! Please correct it! Error: {1}", metaFilePath, e.what());
			return;
		}

		YAML::Node metaRoot = root["Metadata"];

		if (!metaRoot["assetHandle"])
		{
			VT_CORE_CRITICAL("[AssetManager] Meta file {0} is missing an asset handle! Please correct it!", metaFilePath);
			return;
		}

		AssetHandle assetHandle = metaRoot["assetHandle"].as<uint64_t>();

		if (!metaRoot["filePath"])
		{
			VT_CORE_CRITICAL("[AssetManager] Meta file {0} is missing a file path! Please correct it!", metaFilePath);
			return;
		}

		std::filesystem::path filePath = metaRoot["filePath"].as<std::string>();

		std::vector<AssetHandle> dependencies;
		if (metaRoot["Dependencies"])
		{
			for (const auto& d : metaRoot["Dependencies"])
			{
				dependencies.emplace_back(d.as<uint64_t>());
			}
		}

		{
			WriteLock lock{ m_assetRegistryMutex };
			AssetMetadata& metadata = m_assetRegistry[assetHandle];
			metadata.handle = assetHandle;
			metadata.filePath = filePath;
			metadata.dependencies = dependencies;

			VT_DESERIALIZE_PROPERTY(type, *(uint32_t*)&metadata.type, metaRoot, 0);

		}
	}

	std::vector<std::filesystem::path> AssetManager::GetMetaFiles()
	{
		std::vector<std::filesystem::path> files;
		std::string ext(".vtmeta");

		// Project Directory

		if (FileSystem::Exists(ProjectManager::GetAssetsDirectory()))
		{
			for (auto& p : std::filesystem::recursive_directory_iterator(ProjectManager::GetAssetsDirectory()))
			{
				if (p.path().extension() == ext)
				{
					files.emplace_back(p.path());
				}
			}
		}

		// Engine Directory
		for (auto& p : std::filesystem::recursive_directory_iterator(ProjectManager::GetEngineDirectory() / "Engine"))
		{
			if (p.path().extension() == ext)
			{
				files.emplace_back(p.path());
			}
		}

		return files;
	}
}
