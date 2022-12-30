#include "vtpch.h"
#include "AssetManager.h"

#include "Volt/Asset/Importers/AssetImporter.h"

#include "Volt/Asset/Importers/MeshTypeImporter.h"
#include "Volt/Asset/Importers/TextureImporter.h"
#include "Volt/Asset/Importers/MeshSourceImporter.h"
#include "Volt/Asset/Importers/SkeletonImporter.h"
#include "Volt/Asset/Importers/AnimationImporter.h"
#include "Volt/Asset/Importers/SceneImporter.h"
#include "Volt/Asset/Importers/AnimatedCharacterImporter.h"
#include "Volt/Asset/Importers/PrefabImporter.h"

#include "Volt/Core/Base.h"

#include "Volt/Utility/StringUtility.h"
#include "Volt/Utility/FileSystem.h"
#include "Volt/Asset/Importers/ParticlePresetImporter.h"

#include "Volt/Platform/ThreadUtility.h"

#include <yaml-cpp/yaml.h>

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
		myAssetImporters.emplace(AssetType::Scene, CreateScope<SceneImporter>());
		myAssetImporters.emplace(AssetType::Skeleton, CreateScope<SkeletonImporter>());
		myAssetImporters.emplace(AssetType::Animation, CreateScope<AnimationImporter>());
		myAssetImporters.emplace(AssetType::AnimatedCharacter, CreateScope<AnimatedCharacterImporter>());
		myAssetImporters.emplace(AssetType::ParticlePreset, CreateScope<ParticlePresetImporter>());
		myAssetImporters.emplace(AssetType::Prefab, CreateScope<PrefabImporter>());
		myAssetImporters.emplace(AssetType::Font, CreateScope<FontImporter>());
		myAssetImporters.emplace(AssetType::PhysicsMaterial, CreateScope<PhysicsMaterialImporter>());
		myAssetImporters.emplace(AssetType::Video, CreateScope<VideoImporter>());

		LoadAssetRegistry();
		myLoadThread = std::thread(&AssetManager::Thread_LoadAsset, this);
		SetThreadName((DWORD)myLoadThread.native_handle(), "Asset Manager Thread");
	}

	void AssetManager::Shutdown()
	{
		myIsLoadThreadRunning = false;
		myLoadThread.join();

		SaveAssetRegistry();
		TextureImporter::Shutdown();
		MeshTypeImporter::Shutdown();
	}

	void AssetManager::LoadAsset(AssetHandle assetHandle, Ref<Asset>& asset)
	{
		if (myAssetCache.contains(assetHandle))
		{
			asset = myAssetCache.at(assetHandle);
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

			asset->path = path;
			asset->handle = assetHandle;
			myAssetCache.emplace(asset->handle, asset);
		}
	}

	void AssetManager::Unload(AssetHandle assetHandle)
	{
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
		std::scoped_lock lk{ myAssetRegistryMutex };
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

		myAssetImporters[asset->GetType()]->Save(asset);

		if (myAssetRegistry.find(asset->path) == myAssetRegistry.end())
		{
			myAssetRegistry.emplace(asset->path, asset->handle);
		}

		if (myAssetCache.find(asset->handle) == myAssetCache.end())
		{
			myAssetCache.emplace(asset->handle, asset);
		}

		SaveAssetRegistry();
	}

	void AssetManager::MoveAsset(Ref<Asset> asset, const std::filesystem::path& targetDir)
	{
		std::scoped_lock lk{ myAssetRegistryMutex };
		const auto projDir = ProjectManager::GetPath();

		FileSystem::Move(projDir / asset->path, projDir / targetDir);

		const std::filesystem::path newPath = targetDir / asset->path.filename();

		myAssetRegistry.erase(asset->path);
		asset->path = newPath;

		myAssetRegistry.emplace(asset->path, asset->handle);
	}

	void AssetManager::MoveAsset(AssetHandle asset, const std::filesystem::path& targetDir)
	{
		std::scoped_lock lk{ myAssetRegistryMutex };
		const std::filesystem::path oldPath = GetPathFromAssetHandle(asset);
		const std::filesystem::path newPath = targetDir / oldPath.filename();
		const auto projDir = ProjectManager::GetPath();
		
		FileSystem::Move(projDir / oldPath, projDir / targetDir);

		myAssetRegistry.erase(oldPath);
		auto it = myAssetCache.find(asset);
		if (it != myAssetCache.end())
		{
			it->second->path = newPath;
		}
		myAssetRegistry.emplace(newPath, asset);
		SaveAssetRegistry();
	}

	void AssetManager::MoveFolder(const std::filesystem::path& sourceDir, const std::filesystem::path& targetDir)
	{
		if (!targetDir.empty() && !sourceDir.empty())
		{
			std::scoped_lock lk{ myAssetRegistryMutex };
			std::vector<std::filesystem::path> filesToMove{};

			for (const auto& [path, handle] : myAssetRegistry)
			{
				if (path.string().contains(sourceDir.string()))
				{
					filesToMove.emplace_back(path);
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
				myAssetRegistry.erase(p);
			}

			for (const auto& f : filesToAddToRegistry)
			{
				myAssetRegistry.emplace(f);
			}
		}

		SaveAssetRegistry();
	}

	void AssetManager::RenameAsset(AssetHandle asset, const std::string& newName)
	{
		std::scoped_lock lk{ myAssetRegistryMutex };
		const std::filesystem::path oldPath = GetPathFromAssetHandle(asset);
		const std::filesystem::path newPath = oldPath.parent_path() / (newName + oldPath.extension().string());
		const auto projDir = ProjectManager::GetPath();

		FileSystem::Rename(projDir / GetPathFromAssetHandle(asset), newName);

		myAssetRegistry.erase(oldPath);
		auto it = myAssetCache.find(asset);
		if (it != myAssetCache.end())
		{
			it->second->path = newPath;
		}
		myAssetRegistry.emplace(newPath, asset);
		SaveAssetRegistry();
	}

	void AssetManager::RenameAssetFolder(AssetHandle asset, const std::filesystem::path& targetPath)
	{
		std::scoped_lock lk{ myAssetRegistryMutex };
		const std::filesystem::path oldPath = GetPathFromAssetHandle(asset);
		myAssetRegistry.erase(oldPath);
		auto it = myAssetCache.find(asset);
		if (it != myAssetCache.end())
		{
			it->second->path = targetPath;
		}
		myAssetRegistry.emplace(targetPath, asset);
		SaveAssetRegistry();
	}

	void AssetManager::RemoveAsset(AssetHandle asset)
	{
		std::scoped_lock lk{ myAssetRegistryMutex };
		const std::filesystem::path path = GetPathFromAssetHandle(asset);
		const auto projDir = ProjectManager::GetPath();

		myAssetRegistry.erase(path);
		myAssetCache.erase(asset);

		FileSystem::MoveToRecycleBin(projDir / path);
		SaveAssetRegistry();

#ifdef VT_DEBUG
		VT_CORE_INFO("Removing asset {0} with handle {1}!", asset, path.string());
#endif
	}

	void AssetManager::RemoveAsset(const std::filesystem::path& path)
	{
		std::scoped_lock lk{ myAssetRegistryMutex };
		const auto handle = GetAssetHandleFromPath(path);
		myAssetCache.erase(handle);
		myAssetRegistry.erase(path);

		const auto projDir = ProjectManager::GetPath();
		FileSystem::MoveToRecycleBin(projDir / path);
		SaveAssetRegistry();

#ifdef VT_DEBUG
		VT_CORE_INFO("Removing asset {0} with handle {1}!", handle, path.string());
#endif
	}

	void AssetManager::RemoveFromRegistry(AssetHandle asset)
	{
		const std::filesystem::path path = GetPathFromAssetHandle(asset);

		if (!path.empty() && myAssetRegistry.contains(path))
		{
			std::scoped_lock lk{ myAssetRegistryMutex };
			myAssetRegistry.erase(path);
			myAssetCache.erase(asset);

			SaveAssetRegistry();
		}
		else
		{
			VT_CORE_WARN("Asset {0} does not exist in registry!", asset);
		}
	}

	void AssetManager::RemoveFromRegistry(const std::filesystem::path& path)
	{
		if (!path.empty() && myAssetRegistry.contains(path))
		{
			std::scoped_lock lk{ myAssetRegistryMutex };
			const auto handle = GetAssetHandleFromPath(path);

			myAssetCache.erase(handle);
			myAssetRegistry.erase(path);

#ifdef VT_DEBUG
			VT_CORE_INFO("Removing asset {0} with handle {1} from registry!", handle, path.string());
#endif
		}
	}

	void AssetManager::RemoveFolderFromRegistry(const std::filesystem::path& folderPath)
	{
		if (!folderPath.empty())
		{
			std::scoped_lock lk{ myAssetRegistryMutex };
			std::vector<std::filesystem::path> filesToRemove{};

			for (const auto& [path, handle] : myAssetRegistry)
			{
				if (path.string().contains(folderPath.string()))
				{
					filesToRemove.emplace_back(path);
				}
			}

			for (const auto& p : filesToRemove)
			{
				const auto& handle = GetAssetHandleFromPath(p);

				if (myAssetCache.contains(handle))
				{
					myAssetCache.erase(handle);
				}

#ifdef VT_DEBUG
				VT_CORE_INFO("Removing asset {0} with handle {1} from registry!", handle, p.string());
#endif
				myAssetRegistry.erase(p);
			}
		}

		SaveAssetRegistry();
	}

	const AssetHandle AssetManager::AddToRegistry(const std::filesystem::path& path)
	{
		const auto newHandle = AssetHandle{};
		myAssetRegistry.emplace(path, newHandle);
		return newHandle;
	}

	bool AssetManager::IsLoaded(AssetHandle handle) const
	{
		return myAssetCache.contains(handle);
	}

	Ref<Asset> AssetManager::GetAssetRaw(AssetHandle assetHandle)
	{
		auto it = myAssetCache.find(assetHandle);
		if (it != myAssetCache.end())
		{
			return it->second;
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
		if (!Get().myAssetRegistry.contains(path))
		{
			return 0;
		}

		return Get().myAssetRegistry.at(path);
	}

	std::filesystem::path AssetManager::GetPathFromAssetHandle(AssetHandle handle)
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
		return myAssetRegistry.contains(path);
	}

	const std::filesystem::path AssetManager::GetFilesystemPath(AssetHandle handle)
	{
		const auto path = GetPathFromAssetHandle(handle);
		return ProjectManager::GetPath() / path;
	}

	const std::filesystem::path AssetManager::GetRelativePath(const std::filesystem::path& path)
	{
		std::filesystem::path relativePath = path.lexically_normal();
		std::string temp = path.string();
		if (temp.find(ProjectManager::GetPath().string()) != std::string::npos)
		{
			relativePath = std::filesystem::relative(path, ProjectManager::GetPath());
			if (relativePath.empty())
			{
				relativePath = path.lexically_normal();
			}
		}

		return relativePath;
	}

	void AssetManager::QueueAssetInternal(const std::filesystem::path& path, Ref<Asset>& asset)
	{
		std::scoped_lock lk{ myAssetRegistryMutex };
		AssetHandle handle = Asset::Null();

		// Check if asset is loaded
		{
			if (myAssetRegistry.find(path) != myAssetRegistry.end())
			{
				handle = myAssetRegistry.at(path);
			}

			if (handle != Asset::Null() && myAssetCache.find(handle) != myAssetCache.end())
			{
				asset = myAssetCache[handle];
				return;
			}
		}

		if (handle != Asset::Null())
		{
			std::scoped_lock lock(myLoadMutex);
			asset->handle = handle;
			myAssetCache.emplace(handle, asset);
		}

		// If not, queue
		{
			std::scoped_lock lock(myLoadMutex);
			myLoadQueue.emplace_back(LoadJob{ handle, path });

#ifdef VT_DEBUG
			VT_CORE_INFO("Queued asset {0}", path.string());
#endif
		}
	}

	void AssetManager::QueueAssetInternal(AssetHandle assetHandle, Ref<Asset>& asset)
	{
		auto it = myAssetCache.find(assetHandle);
		if (it != myAssetCache.end())
		{
			asset = it->second;
			return;
		}

		const auto path = GetPathFromAssetHandle(assetHandle);
		if (!path.empty())
		{
			QueueAssetInternal(path, asset);
		}
	}

	void AssetManager::Thread_LoadAsset()
	{
		VT_PROFILE_THREAD("Asset Importer Thead");
		myIsLoadThreadRunning = true;
		while (myIsLoadThreadRunning)
		{
			if (!myLoadQueue.empty())
			{

				LoadJob job;

				{
					std::scoped_lock<std::mutex> lock(myLoadMutex);

					job = myLoadQueue.back();
					myLoadQueue.pop_back();
				}

				const auto type = GetAssetTypeFromPath(job.path);
				if (myAssetImporters.find(type) == myAssetImporters.end())
				{
					VT_CORE_ERROR("No importer for asset found!");
					continue;
				}

				VT_PROFILE_SCOPE(std::format("Import {0}", job.path.string()).c_str());

				Ref<Asset> asset;
				asset = myAssetCache.at(job.handle);

				myAssetImporters.at(type)->Load(job.path, asset);
				if (job.handle != Asset::Null())
				{
					asset->handle = job.handle;
				}

#ifdef VT_DEBUG
				VT_CORE_INFO("Loaded asset {0} with handle {1}!", job.path.string().c_str(), asset->handle);
#endif

				asset->SetFlag(AssetFlag::Queued, false);

				{
					std::scoped_lock<std::mutex> lock(myLoadMutex);

					myAssetCache[job.handle] = asset;

					if (job.handle == Asset::Null())
					{
						myAssetRegistry.emplace(job.path, asset->handle);
					}
				}
			}
		}
	}

	void AssetManager::SaveAssetRegistry()
	{
		std::map<AssetHandle, std::string> sortedRegistry;
		for (auto& [path, handle] : myAssetRegistry)
		{
			if (IsSourceFile(handle))
			{
				continue;
			}

			std::string pathToSerialize = path.string();
			std::replace(pathToSerialize.begin(), pathToSerialize.end(), '\\', '/');
			sortedRegistry[handle] = pathToSerialize;
		}

		YAML::Emitter out;
		out << YAML::BeginMap;

		out << YAML::Key << "Assets" << YAML::BeginSeq;
		for (const auto& [handle, path] : sortedRegistry)
		{
			out << YAML::BeginMap;
			out << YAML::Key << "Handle" << YAML::Value << handle;
			out << YAML::Key << "Path" << YAML::Value << path;
			out << YAML::EndMap;
		}
		out << YAML::EndSeq;
		out << YAML::EndMap;

		const auto regPath = Volt::ProjectManager::GetAssetRegistryPath();
		std::ofstream fout(regPath);
		fout << out.c_str();
		fout.close();
	}

	void AssetManager::LoadAssetRegistry()
	{
		if (!std::filesystem::exists(Volt::ProjectManager::GetAssetRegistryPath()))
		{
			return;
		}

		std::ifstream file(Volt::ProjectManager::GetAssetRegistryPath());
		if (!file.is_open()) [[unlikely]]
		{
			VT_CORE_CRITICAL("Failed to open asset registry file: {0}!", Volt::ProjectManager::GetAssetRegistryPath().string().c_str());
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

		YAML::Node assets = root["Assets"];

		for (const auto entry : assets)
		{
			AssetHandle handle = entry["Handle"].as<uint64_t>();

			if (!entry["Path"])
			{
				VT_CORE_CRITICAL("[AssetRegistry] Asset with handle {0} is not formatted correctly! Please correct it!", handle);
				system("pause");
				exit(1);
			}

			std::string path = entry["Path"].as<std::string>();

			if (myAssetRegistry.contains(path))
			{
				VT_CORE_ERROR("[AssetRegistry] Asset {0} with handle {1} is a duplicate! Skipping!", path, handle);
			}
			else
			{
				myAssetRegistry.emplace(path, handle);
			}
		}
	}
}