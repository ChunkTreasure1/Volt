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
	static const std::filesystem::path s_assetRegistryPath = "Assets/AssetRegistry.vtreg";

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

	void AssetManager::LoadAsset(const std::filesystem::path& path, Ref<Asset>& asset)
	{
		AssetHandle handle = Asset::Null();
		if (myAssetRegistry.contains(path))
		{
			handle = myAssetRegistry.at(path);
		}

		if (handle != Asset::Null() && myAssetCache.contains(handle))
		{
			asset = myAssetCache[handle];
			return;
		}

		const auto type = GetAssetTypeFromPath(path);

		if (myAssetImporters.find(type) == myAssetImporters.end())
		{
			VT_CORE_ERROR("No importer for asset found!");
			return;
		}

		myAssetImporters[type]->Load(path, asset);
		if (handle != Asset::Null())
		{
			asset->handle = handle;
		}
		else
		{
			AssetHandle newHandle{};

			myAssetRegistry.emplace(path, newHandle);
			asset->handle = newHandle;
		}

#ifdef VT_DEBUG
		VT_CORE_INFO("Loaded asset {0} with handle {1}!", path.string().c_str(), asset->handle);
#endif

		asset->path = path;
		myAssetCache.emplace(asset->handle, asset);
	}

	void AssetManager::LoadAsset(AssetHandle assetHandle, Ref<Asset>& asset)
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
			LoadAsset(path, asset);
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
		FileSystem::Move(asset->path, targetDir);

		const std::filesystem::path newPath = targetDir / asset->path.filename();

		myAssetRegistry.erase(asset->path);
		asset->path = newPath;

		myAssetRegistry.emplace(asset->path, asset->handle);
	}

	void AssetManager::MoveAsset(AssetHandle asset, const std::filesystem::path& targetDir)
	{
		const std::filesystem::path oldPath = GetPathFromAssetHandle(asset);
		const std::filesystem::path newPath = targetDir / oldPath.filename();
		FileSystem::Move(oldPath, targetDir);

		myAssetRegistry.erase(oldPath);
		auto it = myAssetCache.find(asset);
		if (it != myAssetCache.end())
		{
			it->second->path = newPath;
		}
		myAssetRegistry.emplace(newPath, asset);
		SaveAssetRegistry();
	}

	void AssetManager::RenameAsset(AssetHandle asset, const std::string& newName)
	{
		const std::filesystem::path oldPath = GetPathFromAssetHandle(asset);
		const std::filesystem::path newPath = oldPath.parent_path() / (newName + oldPath.extension().string());
		FileSystem::Rename(GetPathFromAssetHandle(asset), newName);

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
		const std::filesystem::path path = GetPathFromAssetHandle(asset);
		myAssetRegistry.erase(path);
		myAssetCache.erase(asset);

		FileSystem::Remove(path);
		SaveAssetRegistry();
	}

	void AssetManager::RemoveAsset(const std::filesystem::path& path)
	{
		myAssetCache.erase(GetAssetHandleFromPath(path));
		myAssetRegistry.erase(path);

		FileSystem::Remove(path);
		SaveAssetRegistry();
	}

	void AssetManager::RemoveFromRegistry(AssetHandle asset)
	{
		const std::filesystem::path path = GetPathFromAssetHandle(asset);

		if (!path.empty() && myAssetRegistry.contains(path))
		{
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
			myAssetCache.erase(GetAssetHandleFromPath(path));
			myAssetRegistry.erase(path);
		}
	}

	void AssetManager::RemoveFolderFromRegistry(const std::filesystem::path& folderPath)
	{
		if (!folderPath.empty())
		{
			for (const auto& [path, handle] : myAssetRegistry)
			{
				if (path.string().contains(folderPath.string()))
				{
					myAssetCache.erase(GetAssetHandleFromPath(path));
					myAssetRegistry.erase(path);
				}
			}
		}

		SaveAssetRegistry();
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
			Get().myAssetRegistry[path] = AssetHandle{};
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

		return "";
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

	void AssetManager::QueueAssetInternal(const std::filesystem::path& path, Ref<Asset>& asset)
	{
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

				asset->path = job.path;
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
		YAML::Emitter out;
		out << YAML::BeginMap;

		out << YAML::Key << "Assets" << YAML::BeginSeq;
		for (const auto& [path, handle] : myAssetRegistry)
		{
			if (!IsSourceFile(handle))
			{
				out << YAML::BeginMap;
				out << YAML::Key << "Handle" << YAML::Value << handle;
				out << YAML::Key << "Path" << YAML::Value << FileSystem::GetPathRelativeToBaseFolder(path).string();
				out << YAML::EndMap;
			}
		}
		out << YAML::EndSeq;
		out << YAML::EndMap;

		std::ofstream fout(s_assetRegistryPath);
		fout << out.c_str();
		fout.close();
	}

	void AssetManager::LoadAssetRegistry()
	{
		if (!std::filesystem::exists(s_assetRegistryPath))
		{
			return;
		}

		std::ifstream file(s_assetRegistryPath);
		if (!file.is_open()) [[unlikely]]
		{
			VT_CORE_CRITICAL("Failed to open asset registry file: {0}!", s_assetRegistryPath.string().c_str());
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