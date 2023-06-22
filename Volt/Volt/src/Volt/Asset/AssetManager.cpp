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

		myAssetImporters.emplace(AssetType::MeshSource, CreateScope<MeshSourceImporter>());
		myAssetImporters.emplace(AssetType::Texture, CreateScope<TextureSourceImporter>());
		myAssetImporters.emplace(AssetType::Shader, CreateScope<ShaderImporter>());
		myAssetImporters.emplace(AssetType::Material, CreateScope<MaterialImporter>());
		myAssetImporters.emplace(AssetType::Mesh, CreateScope<MeshSourceImporter>());
		myAssetImporters.emplace(AssetType::NavMesh, CreateScope<VTNavMeshImporter>());
		myAssetImporters.emplace(AssetType::Scene, CreateScope<SceneImporter>());
		myAssetImporters.emplace(AssetType::Skeleton, CreateScope<SkeletonImporter>());
		myAssetImporters.emplace(AssetType::AnimationGraph, CreateScope<AnimationGraphImporter>());
		myAssetImporters.emplace(AssetType::Animation, CreateScope<AnimationImporter>());
		myAssetImporters.emplace(AssetType::AnimatedCharacter, CreateScope<AnimatedCharacterImporter>());
		myAssetImporters.emplace(AssetType::ParticlePreset, CreateScope<ParticlePresetImporter>());
		myAssetImporters.emplace(AssetType::Prefab, CreateScope<PrefabImporter>());
		myAssetImporters.emplace(AssetType::Font, CreateScope<FontImporter>());
		myAssetImporters.emplace(AssetType::PhysicsMaterial, CreateScope<PhysicsMaterialImporter>());
		myAssetImporters.emplace(AssetType::Video, CreateScope<VideoImporter>());
		myAssetImporters.emplace(AssetType::BehaviorGraph, CreateScope<BehaviorTreeImporter>());
		myAssetImporters.emplace(AssetType::BlendSpace, CreateScope<BlendSpaceImporter>());
		myAssetImporters.emplace(AssetType::RenderPipeline, CreateScope<RenderPipelineImporter>());
		myAssetImporters.emplace(AssetType::PostProcessingStack, CreateScope<PostProcessingStackImporter>());
		myAssetImporters.emplace(AssetType::PostProcessingMaterial, CreateScope<PostProcessingMaterialImporter>());
		myAssetImporters.emplace(AssetType::NetContract, CreateScope<NetContractImporter>());

		LoadAssetMetaFiles();
	}

	void AssetManager::Shutdown()
	{
		TextureImporter::Shutdown();
		MeshTypeImporter::Shutdown();
	}

	void AssetManager::AddDependency(AssetHandle asset, const std::filesystem::path& dependency)
	{
		if (myAssetDependencies.contains(asset))
		{
			auto& dependencies = myAssetDependencies.at(asset);
			if (std::find(dependencies.begin(), dependencies.end(), dependency) != dependencies.end())
			{
				return;
			}
		}

		myAssetDependencies[asset].emplace_back(dependency);
	}

	const std::vector<std::filesystem::path>& AssetManager::GetDependencies(AssetHandle asset) const
	{
		static std::vector<std::filesystem::path> emptyDependency;
		if (!myAssetDependencies.contains(asset))
		{
			return emptyDependency;
		}

		return myAssetDependencies.at(asset);
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
			ReadLock lock{ myAssetCacheMutex };
			if (myAssetCache.contains(assetHandle))
			{
				asset = myAssetCache.at(assetHandle);
				return;
			}
		}

		if (myMemoryAssets.contains(assetHandle))
		{
			asset = myMemoryAssets.at(assetHandle);
			return;
		}

		const auto path = GetPathFromAssetHandle(assetHandle);
		if (!path.empty())
		{
			const auto type = GetAssetTypeFromPath(path);
			if (!myAssetImporters.contains(type))
			{
				VT_CORE_WARN("[AssetManager] No importer for asset found!");
				return;
			}

			myAssetImporters.at(type)->Load(path, asset);
#ifdef VT_DEBUG
			VT_CORE_TRACE("Tried loading asset {0} with handle {1}!", path.string(), assetHandle);
#endif	

			if (!asset) { return; }
			asset->path = path;
			asset->handle = assetHandle;

			{
				WriteLock lock{ myAssetCacheMutex };
				myAssetCache.emplace(asset->handle, asset);
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
		WriteLock lock{ myAssetCacheMutex };

		auto it = myAssetCache.find(assetHandle);
		if (it == myAssetCache.end())
		{
			VT_CORE_WARN("Unable to unload asset with handle {0}, it has not been loaded!", assetHandle);
			return;
		}

		myAssetCache.erase(it);
	}

	void AssetManager::ReloadAsset(const std::filesystem::path& path)
	{
		AssetHandle handle = GetAssetHandleFromPath(path);
		if (handle == Asset::Null())
		{
			VT_CORE_ERROR("Asset with path {0} is not loaded!", path.string());
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

	void AssetManager::SaveAsset(const Ref<Asset> asset)
	{
		if (myAssetImporters.find(asset->GetType()) == myAssetImporters.end())
		{
			VT_CORE_ERROR("No exporter for asset found!");
			return;
		}

		if (!asset->IsValid())
		{
			VT_CORE_ERROR("Unable to save invalid asset!");
			return;
		}

		const auto type = asset->GetType();
		myAssetImporters[type]->Save(asset);

		if (myAssetRegistry.find(asset->path) == myAssetRegistry.end())
		{
			WriteLock lock{ myAssetRegistryMutex };
			myAssetRegistry.emplace(asset->path, asset->handle);
		}

		{
			WriteLock lock{ myAssetCacheMutex };

			if (!myAssetCache.contains(asset->handle))
			{
				myAssetCache.emplace(asset->handle, asset);
			}
		}

		if (!HasAssetMetaFile(asset->path))
		{
			SaveAssetMetaFile(asset->path);
		}
	}

	void AssetManager::MoveAsset(Ref<Asset> asset, const std::filesystem::path& targetDir)
	{
		const auto projDir = GetContextPath(targetDir);

		FileSystem::Move(projDir / asset->path, projDir / targetDir);

		const std::filesystem::path newPath = targetDir / asset->path.filename();

		auto oldPath = asset->path;

		{
			WriteLock lock{ myAssetRegistryMutex };
			myAssetRegistry.erase(asset->path);
			asset->path = newPath;

			myAssetRegistry.emplace(asset->path, asset->handle);
		}

		RemoveMetaFile(oldPath);
		SaveAssetMetaFile(newPath);
	}

	void AssetManager::MoveAsset(AssetHandle asset, const std::filesystem::path& targetDir)
	{
		const std::filesystem::path oldPath = GetPathFromAssetHandle(asset);
		const std::filesystem::path newPath = targetDir / oldPath.filename();
		const auto projDir = GetContextPath(targetDir);

		FileSystem::Move(projDir / oldPath, projDir / targetDir);

		{
			WriteLock registryLock{ myAssetRegistryMutex };
			ReadLock cacheLock{ myAssetCacheMutex };

			myAssetRegistry.erase(oldPath);
			auto it = myAssetCache.find(asset);
			if (it != myAssetCache.end())
			{
				it->second->path = newPath;
			}
			myAssetRegistry.emplace(newPath, asset);
		}

		RemoveMetaFile(oldPath);
		SaveAssetMetaFile(newPath);
	}

	void AssetManager::MoveFolder(const std::filesystem::path& sourceDir, const std::filesystem::path& targetDir)
	{
		if (!targetDir.empty() && !sourceDir.empty())
		{
			std::vector<std::filesystem::path> filesToMove{};

			{
				ReadLock lock{ myAssetRegistryMutex };
				for (const auto& [path, handle] : myAssetRegistry)
				{
					if (path.string().contains(sourceDir.string()))
					{
						filesToMove.emplace_back(path);
					}
				}
			}

			std::vector<std::pair<std::filesystem::path, AssetHandle>> filesToAddToRegistry{};

			for (const auto& p : filesToMove)
			{
				const auto& handle = GetAssetHandleFromPath(p);

				std::string newPath = p.string();
				const size_t dirLoc = newPath.find(sourceDir.string());

				newPath.erase(dirLoc, sourceDir.string().length());
				newPath.insert(dirLoc, targetDir.string());

				filesToAddToRegistry.emplace_back(std::make_pair<>(newPath, handle));

				{
					WriteLock lock{ myAssetRegistryMutex };
					myAssetRegistry.erase(p);
				}

				RemoveMetaFile(p);
				SaveAssetMetaFile(newPath);
			}

			{
				WriteLock lock{ myAssetRegistryMutex };
				for (const auto& f : filesToAddToRegistry)
				{
					myAssetRegistry.emplace(f);
				}
			}
		}
	}

	void AssetManager::RenameAsset(AssetHandle asset, const std::string& newName)
	{
		const std::filesystem::path oldPath = GetPathFromAssetHandle(asset);
		const std::filesystem::path newPath = oldPath.parent_path() / (newName + oldPath.extension().string());
		const auto projDir = GetContextPath(oldPath);

		FileSystem::Rename(projDir / oldPath, newName);

		{
			WriteLock registryLock{ myAssetRegistryMutex };
			ReadLock cacheLock{ myAssetCacheMutex };

			myAssetRegistry.erase(oldPath);
			auto it = myAssetCache.find(asset);
			if (it != myAssetCache.end())
			{
				it->second->path = newPath;
			}
			myAssetRegistry.emplace(newPath, asset);
		}

		RemoveMetaFile(oldPath);
		SaveAssetMetaFile(newPath);
	}

	void AssetManager::RenameAssetFolder(AssetHandle asset, const std::filesystem::path& targetPath)
	{
		const std::filesystem::path oldPath = GetPathFromAssetHandle(asset);
		myAssetRegistry.erase(oldPath);

		{
			ReadLock lock{ myAssetCacheMutex };
			auto it = myAssetCache.find(asset);
			if (it != myAssetCache.end())
			{
				it->second->path = targetPath;
			}
		}

		{
			WriteLock lock{ myAssetRegistryMutex };
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
			WriteLock lock{ myAssetRegistryMutex };
			myAssetRegistry.erase(path);
		}

		{
			WriteLock lock{ myAssetCacheMutex };
			myAssetCache.erase(asset);
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
			WriteLock lock{ myAssetCacheMutex };
			myAssetCache.erase(handle);
		}

		{
			WriteLock lock{ myAssetRegistryMutex };
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
				WriteLock lock{ myAssetRegistryMutex };
				myAssetRegistry.erase(pathClean);
			}

			{
				WriteLock lock{ myAssetCacheMutex };
				myAssetCache.erase(asset);
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
				WriteLock lock{ myAssetCacheMutex };
				myAssetCache.erase(handle);
			}

			{
				WriteLock lock{ myAssetRegistryMutex };
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
				ReadLock lock{ myAssetRegistryMutex };
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
					WriteLock lock{ myAssetCacheMutex };
					if (myAssetCache.contains(handle))
					{
						myAssetCache.erase(handle);
					}
				}

#ifdef VT_DEBUG
				VT_CORE_INFO("Removing asset {0} with handle {1} from registry!", handle, p.string());
#endif

				{
					WriteLock lock{ myAssetRegistryMutex };
					myAssetRegistry.erase(p);
				}

				RemoveMetaFile(p);
			}
		}
	}

	const Volt::AssetHandle AssetManager::AddToRegistry(const std::filesystem::path& path, AssetHandle handle /*= 0*/)
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
		ReadLock lock{ Get().myAssetCacheMutex };
		return Get().myAssetCache.contains(handle);
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
			ReadLock lock{ myAssetCacheMutex };
			auto it = myAssetCache.find(assetHandle);
			if (it != myAssetCache.end())
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
		if (!s_assetExtensionsMap.contains(ext)) [[unlikely]]
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
		if (myMemoryAssets.contains(handle))
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
			ReadLock registryLock{ myAssetRegistryMutex };
			if (myAssetRegistry.find(path) != myAssetRegistry.end())
			{
				handle = myAssetRegistry.at(path);
			}

			ReadLock cacheLock{ myAssetCacheMutex };
			if (handle != Asset::Null() && myAssetCache.find(handle) != myAssetCache.end())
			{
				asset = myAssetCache[handle];
				return;
			}
		}

		if (handle != Asset::Null())
		{
			WriteLock lock{ myAssetCacheMutex };
			asset->handle = handle;
			myAssetCache.emplace(handle, asset);
		}

		asset->path = path;

		// If not, queue
		{
			auto& threadPool = Application::GetThreadPool();

			threadPool.SubmitTask([this](const std::filesystem::path& path, AssetHandle handle)
			{
				const auto type = GetAssetTypeFromPath(path);
				if (!myAssetImporters.contains(type))
				{
					VT_CORE_ERROR("No importer for asset found!");
					return;
				}

				Ref<Asset> asset;
				{
					ReadLock lock{ myAssetCacheMutex };
					asset = myAssetCache.at(handle);
				}

				myAssetImporters.at(type)->Load(path, asset);
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
					WriteLock lock{ myAssetCacheMutex };
					myAssetCache[handle] = asset;
				}

				if (handle == Asset::Null())
				{
					WriteLock lock{ myAssetRegistryMutex };
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
			ReadLock lock{ myAssetCacheMutex };
			auto it = myAssetCache.find(assetHandle);
			if (it != myAssetCache.end())
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
			ReadLock registryLock{ myAssetRegistryMutex };
			if (myAssetRegistry.find(path) != myAssetRegistry.end())
			{
				handle = myAssetRegistry.at(path);
			}

			ReadLock cacheLock{ myAssetCacheMutex };
			if (handle != Asset::Null() && myAssetCache.find(handle) != myAssetCache.end())
			{
				asset = myAssetCache[handle];
				return;
			}
		}

		if (handle != Asset::Null())
		{
			WriteLock lock{ myAssetCacheMutex };
			asset->handle = handle;
			myAssetCache.emplace(handle, asset);
		}

		asset->path = path;

		// If not, queue
		{
			auto& threadPool = Application::GetThreadPool();

			threadPool.SubmitTask([this, loadedCallback](const std::filesystem::path& path, AssetHandle handle)
			{
				const auto type = GetAssetTypeFromPath(path);
				if (!myAssetImporters.contains(type))
				{
					VT_CORE_ERROR("No importer for asset found!");
					return;
				}

				Ref<Asset> asset;
				{
					ReadLock lock{ myAssetCacheMutex };
					asset = myAssetCache.at(handle);
				}

				myAssetImporters.at(type)->Load(path, asset);
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
					WriteLock lock{ myAssetCacheMutex };
					myAssetCache[handle] = asset;
				}

				if (handle == Asset::Null())
				{
					WriteLock lock{ myAssetRegistryMutex };
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
			ReadLock lock{ myAssetCacheMutex };
			auto it = myAssetCache.find(assetHandle);
			if (it != myAssetCache.end())
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

		//const wchar_t* fileLPCWSTR = Utils::ToWString(metaPath.string()).c_str();

		//int attr = GetFileAttributes(fileLPCWSTR);

		//if ((attr & FILE_ATTRIBUTE_HIDDEN) == 0)
		//{
		//	SetFileAttributes(fileLPCWSTR, attr | FILE_ATTRIBUTE_HIDDEN);
		//}
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
