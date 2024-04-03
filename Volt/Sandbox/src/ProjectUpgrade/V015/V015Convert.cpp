#include "sbpch.h"
#include "V015Convert.h"

#include <Volt/Project/ProjectManager.h>
#include <Volt/Utility/FileSystem.h>

#include <Volt/Asset/Asset.h>
#include <Volt/Asset/AssetManager.h>
#include <Volt/Asset/AssetFactory.h>
#include <Volt/Asset/Importers/AssetImporter.h>
#include <Volt/Asset/Importers/MeshTypeImporter.h>
#include <Volt/Asset/Importers/TextureImporter.h>
#include <Volt/Asset/Importers/MeshSourceImporter.h>
#include <Volt/Asset/Importers/VTNavMeshImporter.h>
#include <Volt/Asset/Importers/SkeletonImporter.h>
#include <Volt/Asset/Importers/AnimationImporter.h>
#include <Volt/Asset/Importers/SceneImporter.h>
#include <Volt/Asset/Importers/AnimatedCharacterImporter.h>
#include <Volt/Asset/Importers/AnimationGraphImporter.h>
#include <Volt/Asset/Importers/PrefabImporter.h>
#include <Volt/Asset/Importers/BehaviorTreeImporter.h>
#include <Volt/Asset/Importers/NetContractImporter.h>
#include <Volt/Asset/Importers/ParticlePresetImporter.h>

#include <Volt/Utility/StringUtility.h>

#include <yaml-cpp/yaml.h>

namespace V015
{
	inline static std::unordered_map<Volt::AssetHandle, Volt::AssetMetadata> s_oldMetadata;

	inline std::vector<std::filesystem::path> GetEngineMetaFiles()
	{
		std::vector<std::filesystem::path> files;
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

	inline std::vector<std::filesystem::path> GetProjectMetaFiles()
	{
		std::vector<std::filesystem::path> files;
		std::string ext(".vtmeta");

		// Project Directory
		const auto assetsDir = Volt::ProjectManager::GetProject().projectDirectory / Volt::ProjectManager::GetProject().assetsDirectory;

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

	inline Volt::AssetType GetAssetTypeFromExtension(const std::string& extension)
	{
		std::string ext = ::Utility::ToLower(extension);
		if (!Volt::s_assetExtensionsMap.contains(ext))
		{
			return Volt::AssetType::None;
		}

		return Volt::s_assetExtensionsMap.at(ext);
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

		Volt::AssetHandle assetHandle = metaRoot["assetHandle"].as<uint64_t>();

		if (!metaRoot["filePath"])
		{
			VT_CORE_CRITICAL("[AssetManager] Meta file {0} is missing a file path! Please correct it!", metaFilePath);
			return;
		}

		std::filesystem::path filePath = metaRoot["filePath"].as<std::string>();

		std::vector<Volt::AssetHandle> dependencies;
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

		const auto type = GetAssetTypeFromExtension(filePath.extension().string());

		std::filesystem::path oldFilePath = filePath;

		if (!Volt::AssetManager::IsSourceAsset(type))
		{
			filePath.replace_extension(".vtasset");
		}

		// If an asset with that filePath already exists, we append the asset type to the name
		auto metadata = Volt::AssetManager::GetMetadataFromFilePath(filePath);
		if (metadata.IsValid())
		{
			std::string newFileName = filePath.stem().string();
			newFileName += "_" + Utility::ReplaceCharacter(Volt::GetAssetTypeName(type), ' ', '_');
			newFileName += filePath.extension().string();

			filePath = filePath.parent_path() / newFileName;
		}

		Volt::AssetMetadata& oldMetadata = s_oldMetadata[assetHandle];
		oldMetadata.handle = assetHandle;
		oldMetadata.filePath = oldFilePath;
		oldMetadata.type = type;

		Volt::AssetManager::Get().AddAssetToRegistry(filePath, assetHandle, type);
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

		std::unordered_map<Volt::AssetType, Scope<Volt::AssetImporter>> importers;
		{
			importers.emplace(Volt::AssetType::ShaderDefinition, CreateScope<Volt::ShaderDefinitionImporter>()); 
			importers.emplace(Volt::AssetType::TextureSource, CreateScope<Volt::TextureSourceImporter>()); 
			importers.emplace(Volt::AssetType::Mesh, CreateScope<Volt::MeshSourceImporter>());
			importers.emplace(Volt::AssetType::NavMesh, CreateScope<Volt::VTNavMeshImporter>());
			importers.emplace(Volt::AssetType::Scene, CreateScope<Volt::SceneImporter>());
			importers.emplace(Volt::AssetType::Skeleton, CreateScope<Volt::SkeletonImporter>());
			importers.emplace(Volt::AssetType::AnimationGraph, CreateScope<Volt::AnimationGraphImporter>());
			importers.emplace(Volt::AssetType::Animation, CreateScope<Volt::AnimationImporter>());
			importers.emplace(Volt::AssetType::AnimatedCharacter, CreateScope<Volt::AnimatedCharacterImporter>());
			importers.emplace(Volt::AssetType::ParticlePreset, CreateScope<Volt::ParticlePresetImporter>());
			importers.emplace(Volt::AssetType::Prefab, CreateScope<Volt::PrefabImporter>());
			importers.emplace(Volt::AssetType::PhysicsMaterial, CreateScope<Volt::PhysicsMaterialImporter>());
			importers.emplace(Volt::AssetType::BehaviorGraph, CreateScope<Volt::BehaviorTreeImporter>());
			importers.emplace(Volt::AssetType::BlendSpace, CreateScope<Volt::BlendSpaceImporter>());
			importers.emplace(Volt::AssetType::PostProcessingStack, CreateScope<Volt::PostProcessingStackImporter>());
			importers.emplace(Volt::AssetType::PostProcessingMaterial, CreateScope<Volt::PostProcessingMaterialImporter>());
			importers.emplace(Volt::AssetType::NetContract, CreateScope<Volt::NetContractImporter>());
		}

		// We start by converting all texture source files to texture files
		for (auto& [handle, metadata] : Volt::AssetManager::GetAssetRegistryMutable())
		{
			if (metadata.type != Volt::AssetType::TextureSource)
			{
				continue;
			}

			Ref<Volt::Asset> asset = Volt::AssetManager::Get().GetFactory().CreateAssetOfType(Volt::AssetType::Texture);
			asset->handle = metadata.handle;
			importers[metadata.type]->Load(s_oldMetadata[metadata.handle], asset);
			asset->handle = metadata.handle;

			metadata.filePath.replace_extension(".vtasset");
			metadata.type = Volt::AssetType::Texture;
			Volt::AssetManager::SaveAsset(asset);
		}

		// Then we resave all other assets into it's new format
		for (const auto& [handle, metadata] : Volt::AssetManager::GetAssetRegistry())
		{
			if (!importers.contains(metadata.type) || metadata.type == Volt::AssetType::ShaderDefinition || metadata.type == Volt::AssetType::TextureSource)
			{
				continue;
			}

			Ref<Volt::Asset> asset = Volt::AssetManager::Get().GetFactory().CreateAssetOfType(metadata.type);
			asset->handle = metadata.handle;
			importers[metadata.type]->Load(s_oldMetadata[metadata.handle], asset);
			asset->handle = metadata.handle;

			Volt::AssetManager::SaveAsset(asset);
		}

		s_oldMetadata.clear();
	}
}
