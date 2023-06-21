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

#include "Volt/Utility/FileSystem.h"
#include "Volt/Asset/Importers/ParticlePresetImporter.h"

#include "Volt/Platform/ThreadUtility.h"

#include "Volt/Utility/YAMLSerializationHelpers.h"
#include "Volt/Asset/Importers/NetContractImporter.h"

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
		m_assetImporters.emplace(AssetType::RenderPipeline, CreateScope<RenderPipelineImporter>());
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
		auto& metaData = GetMetaDataFromHandleMutable(asset);
		const auto& dependencyMetaData = GetMetaDataFromFilePath(dependency);

		AddDependency(asset, dependencyMetaData.handle);
	}

	void AssetManager::AddDependency(AssetHandle asset, AssetHandle dependency)
	{
		auto& metaData = GetMetaDataFromHandleMutable(asset);
		const auto& dependencyMetaData = GetMetaDataFromHandle(dependency);

		if (!metaData.IsValid())
		{
			VT_CORE_WARN("[AssetManager] Trying to add dependency {0} to invalid asset {1}", dependency, asset);
			return;
		}

		if (!dependencyMetaData.IsValid())
		{
			VT_CORE_WARN("[AssetManager] Trying to add invalid dependency {0} to asset {1}", dependency, asset);
			return;
		}

		auto it = std::find_if(metaData.dependencies.begin(), metaData.dependencies.end(), [&](AssetHandle dep)
		{
			return dep == dependencyMetaData.handle;
		});

		if (it != metaData.dependencies.end())
		{
			return;
		}

		metaData.dependencies.emplace_back(dependencyMetaData.handle);
	}

	const std::vector<AssetHandle> AssetManager::GetAllAssetsWithDependency(const std::filesystem::path& dependencyPath)
	{
		const std::string pathString = Utils::ReplaceCharacter(Get().GetRelativePath(dependencyPath).string(), '\\', '/');
		std::vector<AssetHandle> result{};

		for (const auto& [handle, dependencies] : Get().myAssetDependencies)
		{
			if (std::find(dependencies.begin(), dependencies.end(), pathString) != dependencies.end())
			{
				result.push_back(handle);
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

		const auto path = GetPathFromAssetHandle(assetHandle);
		if (!path.empty())
		{
			const auto type = GetAssetTypeFromPath(path);
			if (!m_assetImporters.contains(type))
			{
				VT_CORE_WARN("[AssetManager] No importer for asset found!");
				return;
			}

			m_assetImporters.at(type)->Load(path, asset);
#ifdef VT_DEBUG
			VT_CORE_TRACE("[AssetManager] Tried loading asset {0} with handle {1}!", path.string(), assetHandle);
#endif	

			if (!asset) { return; }
			asset->handle = assetHandle;

			{
				WriteLock lock{ m_assetCacheMutex };
				m_assetCache.emplace(asset->handle, asset);
			}
		}
	}

	void AssetManager::LoadAssetMetaFiles()
	{
		for (auto file : GetMetaFiles())
		{
			LoadAssetMetaFile(file);
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
		AssetHandle handle = GetAssetHandleFromPath(path);
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

			AssetMetaData& metaData = instance.m_assetRegistry[asset->handle];
			metaData.filePath = targetFilePath;
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

		instance.SaveAssetMetaFile(targetFilePath);
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

		if (!instance.m_assetRegistry.contains(asset->handle))
		{
			VT_CORE_ERROR("[AssetManager] Unable to save asset which has not been saved before!");
			return;
		}

		const auto type = asset->GetType();
		instance.m_assetImporters[type]->Save(asset);

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

			const auto& assetMetaData = GetMetaDataFromHandle(asset->handle);
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
		SaveAssetMetaFile(newPath);
	}

	void AssetManager::MoveAsset(AssetHandle assetHandle, const std::filesystem::path& targetDir)
	{
		std::filesystem::path assetFilePath;

		{
			ReadLock lock{ m_assetRegistryMutex };

			const auto& assetMetaData = GetMetaDataFromHandle(assetHandle);
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
		SaveAssetMetaFile(newPath);
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
				auto& metaData = GetMetaDataFromHandleMutable(handle);

				std::string newPath = metaData.filePath.string();
				const size_t directoryStringLoc = Utils::ToLower(newPath).find(Utils::ToLower(sourceDir.string()));

				if (directoryStringLoc == std::string::npos)
				{
					continue;
				}

				newPath.erase(directoryStringLoc, sourceDir.string().length());
				newPath.insert(directoryStringLoc, targetDir.string());

				RemoveMetaFile(metaData.filePath);
				metaData.filePath = newPath;

				SaveAssetMetaFile(newPath);
			}
		}
	}

	void AssetManager::RenameAsset(AssetHandle asset, const std::string& newName)
	{
		const std::filesystem::path oldPath = GetPathFromAssetHandle(asset);
		const std::filesystem::path newPath = oldPath.parent_path() / (newName + oldPath.extension().string());
		const auto projDir = GetContextPath(oldPath);

		{
			WriteLock lock{ m_assetRegistryMutex };
			if (!m_assetRegistry.contains(asset))
			{
				VT_CORE_WARN("[AssetManager] Trying to rename invalid asset!");
				return;
			}

			m_assetRegistry.at(asset).filePath = newPath;
		}

		FileSystem::Rename(projDir / oldPath, newName);

		RemoveMetaFile(oldPath);
		SaveAssetMetaFile(newPath);
	}

	void AssetManager::RenameAssetFolder(AssetHandle asset, const std::filesystem::path& targetPath)
	{
		const std::filesystem::path oldPath = GetPathFromAssetHandle(asset);
		myAssetRegistry.erase(oldPath);

		{
			ReadLock lock{ m_assetCacheMutex };
			auto it = m_assetCache.find(asset);
			if (it != m_assetCache.end())
			{
				it->second->path = targetPath;
			}
		}

		{
			WriteLock lock{ m_assetRegistryMutex };
			myAssetRegistry.emplace(targetPath, asset);
		}

		RemoveMetaFile(oldPath);
		SaveAssetMetaFile(targetPath);
	}

	void AssetManager::RemoveAsset(AssetHandle asset)
	{
		const std::filesystem::path path = GetPathFromAssetHandle(asset);
		const auto projDir = GetContextPath(path);

		{
			WriteLock lock{ m_assetRegistryMutex };
			myAssetRegistry.erase(path);
		}

		{
			WriteLock lock{ m_assetCacheMutex };
			m_assetCache.erase(asset);
		}

		FileSystem::MoveToRecycleBin(projDir / path);

		RemoveMetaFile(path);

#ifdef VT_DEBUG
		VT_CORE_INFO("Removing asset {0} with handle {1}!", asset, path.string());
#endif
	}

	void AssetManager::RemoveAsset(const std::filesystem::path& path)
	{
		const auto handle = GetAssetHandleFromPath(path);

		{
			WriteLock lock{ m_assetCacheMutex };
			m_assetCache.erase(handle);
		}

		{
			WriteLock lock{ m_assetRegistryMutex };
			myAssetRegistry.erase(path);
		}

		const auto projDir = GetContextPath(path);
		FileSystem::MoveToRecycleBin(projDir / path);

		RemoveMetaFile(path);

#ifdef VT_DEBUG
		VT_CORE_INFO("Removing asset {0} with handle {1}!", handle, path.string());
#endif
	}

	void AssetManager::RemoveMetaFile(const std::filesystem::path& path)
	{
		auto metafile = GetContextPath(path) / path;
		metafile.replace_filename(metafile.filename().string() + ".vtmeta");
		if (std::filesystem::exists(metafile))
		{
			std::filesystem::remove(metafile);
		}
	}

	void AssetManager::RemoveFromRegistry(AssetHandle asset)
	{
		const std::filesystem::path path = GetPathFromAssetHandle(asset);
		auto pathClean = GetCleanPath(path);

		if (!path.empty() && myAssetRegistry.contains(pathClean))
		{
			{
				WriteLock lock{ m_assetRegistryMutex };
				myAssetRegistry.erase(pathClean);
			}

			{
				WriteLock lock{ m_assetCacheMutex };
				m_assetCache.erase(asset);
			}

			RemoveMetaFile(pathClean);
		}
		else
		{
			VT_CORE_WARN("Asset {0} does not exist in registry!", asset);
		}
	}

	void AssetManager::RemoveFromRegistry(const std::filesystem::path& path)
	{
		auto pathClean = GetCleanPath(path);

		if (!pathClean.empty() && myAssetRegistry.contains(pathClean))
		{
			const auto handle = GetAssetHandleFromPath(pathClean);

			{
				WriteLock lock{ m_assetCacheMutex };
				m_assetCache.erase(handle);
			}

			{
				WriteLock lock{ m_assetRegistryMutex };
				myAssetRegistry.erase(pathClean);
			}

			RemoveMetaFile(pathClean);

#ifdef VT_DEBUG
			VT_CORE_INFO("Removing asset {0} with handle {1} from registry!", handle, pathClean);
#endif
		}
	}

	void AssetManager::RemoveFolderFromRegistry(const std::filesystem::path& folderPath)
	{
		if (!folderPath.empty())
		{
			std::vector<std::filesystem::path> filesToRemove{};

			{
				ReadLock lock{ m_assetRegistryMutex };
				for (const auto& [path, handle] : myAssetRegistry)
				{
					if (path.string().contains(folderPath.string()))
					{
						filesToRemove.emplace_back(path);
					}
				}
			}

			for (const auto& p : filesToRemove)
			{
				const auto& handle = GetAssetHandleFromPath(p);

				{
					WriteLock lock{ m_assetCacheMutex };
					if (m_assetCache.contains(handle))
					{
						m_assetCache.erase(handle);
					}
				}

#ifdef VT_DEBUG
				VT_CORE_INFO("Removing asset {0} with handle {1} from registry!", handle, p.string());
#endif

				{
					WriteLock lock{ m_assetRegistryMutex };
					myAssetRegistry.erase(p);
				}

				RemoveMetaFile(p);
			}
		}
	}

	const Volt::AssetHandle AssetManager::AddAssetToRegistry(const std::filesystem::path& path, AssetHandle handle /*= 0*/)
	{
		auto pathClean = GetCleanPath(path);

		if (myAssetRegistry.contains(pathClean))
		{
			return myAssetRegistry.at(pathClean);
		}

		VT_ASSERT(GetPathFromAssetHandle(handle).empty(), "Handle already in use");

		const auto newHandle = (handle) ? handle : AssetHandle{};

		myAssetRegistry.emplace(pathClean, newHandle);

		VT_CORE_ASSERT(!HasAssetMetaFile(pathClean), "Asset has a meta file! AddToRegister should not be called on this file!");

		if (!HasAssetMetaFile(pathClean))
		{
			SaveAssetMetaFile(pathClean);
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
		return GetAssetTypeFromExtension(GetPathFromAssetHandle(handle).extension().string());
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

	AssetHandle AssetManager::GetAssetHandleFromPath(const std::filesystem::path& path)
	{
		auto pathClean = GetCleanPath(path);

		if (!Get().myAssetRegistry.contains(pathClean))
		{
			if (IsEngineAsset(path))
			{
				AssetHandle newHandle{};
				Get().myAssetRegistry.emplace(pathClean, newHandle);

				if (!Get().HasAssetMetaFile(pathClean))
				{
					Get().SaveAssetMetaFile(pathClean);
				}

				return newHandle;
			}

			return 0;
		}

		return Get().myAssetRegistry.at(pathClean);
	}

	const AssetMetaData& AssetManager::GetMetaDataFromHandle(AssetHandle handle)
	{
		auto& instance = Get();

		if (!instance.m_assetRegistry.contains(handle))
		{
			return s_nullMetaData;
		}

		return instance.m_assetRegistry.at(handle);
	}

	const AssetMetaData& AssetManager::GetMetaDataFromFilePath(const std::filesystem::path filePath)
	{
		auto& instance = Get();

		for (const auto& [handle, metaData] : instance.m_assetRegistry)
		{
			if (metaData.filePath == filePath)
			{
				return metaData;
			}
		}

		return s_nullMetaData;
	}

	const std::filesystem::path AssetManager::GetPathFromAssetHandle(AssetHandle handle)
	{
		for (const auto& [path, asset] : Get().myAssetRegistry)
		{
			if (asset == handle)
			{
				return path;
			}
		}

		return {};
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

	std::vector<std::filesystem::path> AssetManager::GetPathFromFilename(std::string filename)
	{
		std::vector<std::filesystem::path> paths;
		for (const auto& [path, asset] : Get().myAssetRegistry)
		{
			if (path.filename() == filename)
			{
				paths.emplace_back(path);
			}
		}
		return paths;
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

	bool AssetManager::ExistsInRegistry(AssetHandle handle) const
	{
		if (m_memoryAssets.contains(handle))
		{
			return true;
		}

		for (const auto& [path, h] : myAssetRegistry)
		{
			if (h == handle)
			{
				return true;
			}
		}

		return false;
	}

	bool AssetManager::ExistsInRegistry(const std::filesystem::path& path) const
	{
		auto pathClean = GetCleanPath(path);
		return myAssetRegistry.contains(pathClean);
	}

	const std::filesystem::path AssetManager::GetFilesystemPath(AssetHandle handle)
	{
		const auto path = GetPathFromAssetHandle(handle);
		return GetContextPath(path) / path;
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

	void AssetManager::QueueAssetInternal(const std::filesystem::path& path, Ref<Asset>& asset)
	{
		AssetHandle handle = Asset::Null();

		// Check if asset is loaded
		{
			ReadLock registryLock{ m_assetRegistryMutex };
			if (myAssetRegistry.find(path) != myAssetRegistry.end())
			{
				handle = myAssetRegistry.at(path);
			}

			ReadLock cacheLock{ m_assetCacheMutex };
			if (handle != Asset::Null() && m_assetCache.find(handle) != m_assetCache.end())
			{
				asset = m_assetCache[handle];
				return;
			}
		}

		if (handle != Asset::Null())
		{
			WriteLock lock{ m_assetCacheMutex };
			asset->handle = handle;
			m_assetCache.emplace(handle, asset);
		}

		asset->path = path;

		// If not, queue
		{
			auto& threadPool = Application::GetThreadPool();

			threadPool.SubmitTask([this](const std::filesystem::path& path, AssetHandle handle)
			{
				const auto type = GetAssetTypeFromPath(path);
				if (!m_assetImporters.contains(type))
				{
					VT_CORE_ERROR("No importer for asset found!");
					return;
				}

				Ref<Asset> asset;
				{
					ReadLock lock{ m_assetCacheMutex };
					asset = m_assetCache.at(handle);
				}

				m_assetImporters.at(type)->Load(path, asset);
				if (handle != Asset::Null())
				{
					asset->handle = handle;
				}

				asset->path = path;

#ifndef VT_DIST
				VT_CORE_INFO("Loaded asset {0} with handle {1}!", path.string().c_str(), asset->handle);
#endif

				asset->SetFlag(AssetFlag::Queued, false);

				{
					WriteLock lock{ m_assetCacheMutex };
					m_assetCache[handle] = asset;
				}

				if (handle == Asset::Null())
				{
					WriteLock lock{ m_assetRegistryMutex };
					myAssetRegistry.emplace(path, asset->handle);
				}

			}, path, handle);

#ifndef VT_DIST
			VT_CORE_TRACE("Queued asset {0}", path.string());
#endif
		}
	}

	void AssetManager::QueueAssetInternal(AssetHandle assetHandle, Ref<Asset>& asset)
	{
		{
			ReadLock lock{ m_assetCacheMutex };
			auto it = m_assetCache.find(assetHandle);
			if (it != m_assetCache.end())
			{
				asset = it->second;
				return;
			}
		}

		const auto path = GetPathFromAssetHandle(assetHandle);
		if (!path.empty())
		{
			QueueAssetInternal(path, asset);
		}
	}

	void AssetManager::QueueAssetInternal(const std::filesystem::path& path, Ref<Asset>& asset, const std::function<void()>& loadedCallback)
	{
		AssetHandle handle = Asset::Null();

		// Check if asset is loaded
		{
			ReadLock registryLock{ m_assetRegistryMutex };
			if (myAssetRegistry.find(path) != myAssetRegistry.end())
			{
				handle = myAssetRegistry.at(path);
			}

			ReadLock cacheLock{ m_assetCacheMutex };
			if (handle != Asset::Null() && m_assetCache.find(handle) != m_assetCache.end())
			{
				asset = m_assetCache[handle];
				return;
			}
		}

		if (handle != Asset::Null())
		{
			WriteLock lock{ m_assetCacheMutex };
			asset->handle = handle;
			m_assetCache.emplace(handle, asset);
		}

		asset->path = path;

		// If not, queue
		{
			auto& threadPool = Application::GetThreadPool();

			threadPool.SubmitTask([this, loadedCallback](const std::filesystem::path& path, AssetHandle handle)
			{
				const auto type = GetAssetTypeFromPath(path);
				if (!m_assetImporters.contains(type))
				{
					VT_CORE_ERROR("No importer for asset found!");
					return;
				}

				Ref<Asset> asset;
				{
					ReadLock lock{ m_assetCacheMutex };
					asset = m_assetCache.at(handle);
				}

				m_assetImporters.at(type)->Load(path, asset);
				if (handle != Asset::Null())
				{
					asset->handle = handle;
				}

				asset->path = path;

#ifndef VT_DIST
				VT_CORE_INFO("Loaded asset {0} with handle {1}!", path.string().c_str(), asset->handle);
#endif

				asset->SetFlag(AssetFlag::Queued, false);

				{
					WriteLock lock{ m_assetCacheMutex };
					m_assetCache[handle] = asset;
				}

				if (handle == Asset::Null())
				{
					WriteLock lock{ m_assetRegistryMutex };
					myAssetRegistry.emplace(path, asset->handle);
				}

				loadedCallback();

			}, path, handle);

#ifndef VT_DIST
			VT_CORE_TRACE("Queued asset {0}", path.string());
#endif
		}
	}

	void AssetManager::QueueAssetInternal(AssetHandle assetHandle, Ref<Asset>& asset, const std::function<void()>& loadedCallback)
	{
		{
			ReadLock lock{ m_assetCacheMutex };
			auto it = m_assetCache.find(assetHandle);
			if (it != m_assetCache.end())
			{
				asset = it->second;
				return;
			}
		}

		const auto path = GetPathFromAssetHandle(assetHandle);
		if (!path.empty())
		{
			QueueAssetInternal(path, asset, loadedCallback);
		}
	}

	AssetMetaData& AssetManager::GetMetaDataFromHandleMutable(AssetHandle handle)
	{
		auto& instance = Get();

		if (!instance.m_assetRegistry.contains(handle))
		{
			return s_nullMetaData;
		}

		return instance.m_assetRegistry.at(handle);
	}

	AssetMetaData& AssetManager::GetMetaDataFromFilePathMutable(const std::filesystem::path filePath)
	{
		auto& instance = Get();

		for (auto& [handle, metaData] : instance.m_assetRegistry)
		{
			if (metaData.filePath == filePath)
			{
				return metaData;
			}
		}

		return s_nullMetaData;
	}

	const std::filesystem::path AssetManager::GetCleanPath(const std::filesystem::path& path)
	{
		auto pathClean = Utils::ReplaceCharacter(path.string(), '\\', '/');
		return pathClean;
	}

	void AssetManager::SaveAssetMetaFile(std::filesystem::path assetPath)
	{
		if (!assetPath.has_stem())
		{
			return;
		}
		auto pathClean = GetCleanPath(assetPath);
		auto handle = GetAssetHandleFromPath(pathClean);

		YAML::Emitter out;
		out << YAML::BeginMap;

		out << YAML::Key << "Handle" << YAML::Value << handle;
		out << YAML::Key << "Path" << YAML::Value << pathClean;

		std::vector<std::string> dependenciesToSerialize;
		for (const auto& d : GetDependencies(handle))
		{
			dependenciesToSerialize.push_back(Utils::ReplaceCharacter(d.string(), '\\', '/'));
		}

		out << YAML::Key << "Dependencies" << YAML::Value << dependenciesToSerialize;
		out << YAML::EndMap;

		auto metaPath = GetContextPath(assetPath) / assetPath;
		metaPath.replace_filename(assetPath.filename().string() + ".vtmeta");

		if (std::filesystem::exists(metaPath))
		{
			return;
		}

		std::ofstream fout(metaPath);
		fout << out.c_str();
		fout.close();
	}

	bool AssetManager::HasAssetMetaFile(const std::filesystem::path& assetPath)
	{
		auto metaPath = GetContextPath(assetPath) / assetPath;
		metaPath.replace_filename(assetPath.filename().string() + ".vtmeta");

		return FileSystem::Exists(metaPath);
	}

	void AssetManager::LoadAssetMetaFile(std::filesystem::path metaPath)
	{
		if (!std::filesystem::exists(metaPath))
		{
			return;
		}

		std::ifstream file(metaPath);
		if (!file.is_open()) [[unlikely]]
		{
			VT_CORE_CRITICAL("Failed to open asset registry file: {0}!", metaPath.string().c_str());
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
			VT_CORE_CRITICAL("Asset Registry contains invalid YAML! Please correct it! Error: {0}", e.what());
			return;
		}

		AssetHandle handle = root["Handle"].as<uint64_t>();

		if (!root["Path"])
		{
			VT_CORE_CRITICAL("[AssetRegistry] Asset with handle {0} is not formatted correctly! Please correct it!", handle);
			system("pause");
			exit(1);
		}

		std::string path = root["Path"].as<std::string>();

		if (myAssetRegistry.contains(path))
		{
			VT_CORE_ERROR("[AssetRegistry] Asset {0} with handle {1} is a duplicate! Skipping!", path, handle);
		}
		else
		{
			myAssetRegistry.emplace(path, handle);
		}

		for (const auto& d : root["Dependencies"])
		{
			AddDependency(handle, d.as<std::string>());
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
