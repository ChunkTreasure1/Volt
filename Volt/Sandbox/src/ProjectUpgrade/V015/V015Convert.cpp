#include "sbpch.h"
#include "V015Convert.h"

#include <Volt/Project/ProjectManager.h>
#include <Volt/Utility/FileSystem.h>

#include <Volt/Asset/Importers/AssetImporter.h>
#include <Volt/Asset/Importers/MeshTypeImporter.h>
#include <Volt/Asset/Importers/TextureImporter.h>
#include <Volt/Asset/Importers/MeshSourceImporter.h>
#include <Volt/Asset/Importers/VTNavMeshImporter.h>
#include <Volt/Asset/Importers/SkeletonImporter.h>
#include <Volt/Asset/Importers/AnimationImporter.h>
#include <Volt/Asset/Importers/SceneImporter.h>
#include <Volt/Asset/Importers/AnimatedCharacterImporter.h>
#include <Volt/Asset/Importers/PrefabImporter.h>
#include <Volt/Asset/Importers/BehaviorTreeImporter.h>
#include <Volt/Asset/Importers/NetContractImporter.h>
#include <Volt/Asset/Importers/ParticlePresetImporter.h>

#include <Volt/Utility/StringUtility.h>

#include <AssetSystem/Asset.h>
#include <AssetSystem/AssetManager.h>
#include <AssetSystem/AssetFactory.h>

#include <yaml-cpp/yaml.h>

namespace V015
{
	inline static std::unordered_map<Volt::AssetHandle, Volt::AssetMetadata> s_oldMetadata;

	inline Vector<std::filesystem::path> GetEngineMetaFiles()
	{
		Vector<std::filesystem::path> files;
		const std::string ext(".vtmeta");

		// Engine Directory
		for (auto& p : std::filesystem::recursive_directory_iterator(Volt::ProjectManager::GetEngineDirectory() / "Engine"))
		{
			if (p.path().extension() == ext)
			{
				files.emplace_back(p.path());
			}
		}

		const auto editorFolder = Volt::ProjectManager::GetEngineDirectory() / "Editor";
		if (FileSystem::Exists(editorFolder))
		{
			for (auto& p : std::filesystem::recursive_directory_iterator(editorFolder))
			{
				if (p.path().extension() == ext)
				{
					files.emplace_back(p.path());
				}
			}
		}

		return files;
	}

	inline Vector<std::filesystem::path> GetProjectMetaFiles()
	{
		Vector<std::filesystem::path> files;
		std::string ext(".vtmeta");

		// Project Directory
		const auto assetsDir = Volt::ProjectManager::GetProject().rootDirectory / Volt::ProjectManager::GetProject().assetsDirectory;

		if (FileSystem::Exists(assetsDir))
		{
			for (auto& p : std::filesystem::recursive_directory_iterator(assetsDir))
			{
				if (p.path().extension() == ext)
				{
					files.emplace_back(p.path());
				}
			}
		}

		return files;
	}

	inline void DeserializeAssetMetafile(std::filesystem::path metaFilePath)
	{
		if (!std::filesystem::exists(metaFilePath))
		{
			return;
		}

		std::ifstream file(metaFilePath);
		if (!file.is_open())
		{
			VT_LOG(Critical, "[AssetManager] Failed to open asset registry file: {0}!", metaFilePath.string().c_str());
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
			VT_LOG(Critical, "[AssetManager] Meta file {0} contains invalid YAML! Please correct it! Error: {1}", metaFilePath, e.what());
			return;
		}

		YAML::Node metaRoot = root["Metadata"];

		if (!metaRoot["assetHandle"])
		{
			VT_LOG(Critical, "[AssetManager] Meta file {0} is missing an asset handle! Please correct it!", metaFilePath);
			return;
		}

		Volt::AssetHandle assetHandle = metaRoot["assetHandle"].as<uint64_t>();

		if (!metaRoot["filePath"])
		{
			VT_LOG(Critical, "[AssetManager] Meta file {0} is missing a file path! Please correct it!", metaFilePath);
			return;
		}

		std::filesystem::path filePath = metaRoot["filePath"].as<std::string>();

		Vector<Volt::AssetHandle> dependencies;
		if (metaRoot["Dependencies"])
		{
			for (const auto& d : metaRoot["Dependencies"])
			{
				dependencies.emplace_back(d.as<uint64_t>());
			}
		}

		std::unordered_map<std::string, std::string> assetProperties;

		if (metaRoot["Properties"])
		{
			for (const auto& node : metaRoot["Properties"])
			{
				const auto key = node.first.as<std::string>();
				const auto value = node.as<std::string>();

				assetProperties[key] = value;
			}
		}

		//const auto type = GetAssetTypeFromExtension(filePath.extension().string());

		//std::filesystem::path oldFilePath = filePath;

		//if (!Volt::AssetManager::IsSourceAsset(type))
		//{
		//	filePath.replace_extension(".vtasset");
		//}

		//// If an asset with that filePath already exists, we append the asset type to the name
		//auto metadata = Volt::AssetManager::GetMetadataFromFilePath(filePath);
		//if (metadata.IsValid())
		//{
		//	std::string newFileName = filePath.stem().string();
		//	newFileName += "_" + Utility::ReplaceCharacter(Volt::GetAssetTypeName(type), ' ', '_');
		//	newFileName += filePath.extension().string();

		//	filePath = filePath.parent_path() / newFileName;
		//}

		//Volt::AssetMetadata& oldMetadata = s_oldMetadata[assetHandle];
		//oldMetadata.handle = assetHandle;
		//oldMetadata.filePath = oldFilePath;
		//oldMetadata.type = type;

		//Volt::AssetManager::Get().AddAssetToRegistry(filePath, assetHandle, type);
	}

	void LoadAssetMetadata()
	{
		const auto projectMetaFiles = GetProjectMetaFiles();
		const auto engineMetaFiles = GetEngineMetaFiles();

		for (auto file : engineMetaFiles)
		{
			//DeserializeAssetMetafile(file);
		}

		for (auto file : projectMetaFiles)
		{
			DeserializeAssetMetafile(file);
		}
	}

	void Convert()
	{
		LoadAssetMetadata();

		std::unordered_map<AssetType, Scope<Volt::AssetImporter>> importers;
		{
			importers.emplace(AssetTypes::ShaderDefinition, CreateScope<Volt::ShaderDefinitionImporter>()); 
			importers.emplace(AssetTypes::TextureSource, CreateScope<Volt::TextureSourceImporter>()); 
			importers.emplace(AssetTypes::Mesh, CreateScope<Volt::MeshSourceImporter>());
			importers.emplace(AssetTypes::NavMesh, CreateScope<Volt::VTNavMeshImporter>());
			importers.emplace(AssetTypes::Scene, CreateScope<Volt::SceneImporter>());
			importers.emplace(AssetTypes::Skeleton, CreateScope<Volt::SkeletonImporter>());
			importers.emplace(AssetTypes::Animation, CreateScope<Volt::AnimationImporter>());
			importers.emplace(AssetTypes::AnimatedCharacter, CreateScope<Volt::AnimatedCharacterImporter>());
			importers.emplace(AssetTypes::ParticlePreset, CreateScope<Volt::ParticlePresetImporter>());
			importers.emplace(AssetTypes::Prefab, CreateScope<Volt::PrefabImporter>());
			importers.emplace(AssetTypes::PhysicsMaterial, CreateScope<Volt::PhysicsMaterialImporter>());
			importers.emplace(AssetTypes::BehaviorGraph, CreateScope<Volt::BehaviorTreeImporter>());
			importers.emplace(AssetTypes::BlendSpace, CreateScope<Volt::BlendSpaceImporter>());
			importers.emplace(AssetTypes::NetContract, CreateScope<Volt::NetContractImporter>());
		}

		// We start by converting all texture source files to texture files
		for (auto& [handle, metadata] : Volt::AssetManager::GetAssetRegistryMutable())
		{
			if (metadata.type != AssetTypes::TextureSource)
			{
				continue;
			}

			Ref<Volt::Asset> asset = GetAssetFactory().CreateAssetOfType(AssetTypes::Texture);
			asset->handle = metadata.handle;
			importers[metadata.type]->Load(s_oldMetadata[metadata.handle], asset);
			asset->handle = metadata.handle;

			metadata.filePath.replace_extension(".vtasset");
			metadata.type = AssetTypes::Texture;
			Volt::AssetManager::SaveAsset(asset);
		}

		// Then we resave all other assets into it's new format
		for (const auto& [handle, metadata] : Volt::AssetManager::GetAssetRegistry())
		{
			if (!importers.contains(metadata.type) || metadata.type == AssetTypes::ShaderDefinition || metadata.type == AssetTypes::TextureSource)
			{
				continue;
			}

			Ref<Volt::Asset> asset = GetAssetFactory().CreateAssetOfType(metadata.type);
			asset->handle = metadata.handle;
			importers[metadata.type]->Load(s_oldMetadata[metadata.handle], asset);
			asset->handle = metadata.handle;

			Volt::AssetManager::SaveAsset(asset);
		}

		s_oldMetadata.clear();
	}
}
