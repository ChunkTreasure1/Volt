#include "vtpch.h"
#include "ShaderDefinitionSerializer.h"

#include "Volt/Asset/AssetManager.h"
#include "Volt/Asset/Rendering/ShaderDefinition.h"

#include <CoreUtilities/FileIO/YAMLFileStreamWriter.h>
#include <CoreUtilities/FileIO/YAMLFileStreamReader.h>

namespace Volt
{
	void ShaderDefinitionSerializer::Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) const
	{
		Ref<ShaderDefinition> shaderDef = std::reinterpret_pointer_cast<ShaderDefinition>(asset);
		
		const auto filePath = AssetManager::GetFilesystemPath(metadata.filePath);
		YAMLFileStreamWriter yamlStreamWriter{ filePath };
		yamlStreamWriter.BeginMap();

		// Serialize AssetMetadata
		yamlStreamWriter.SetKey("assetHandle", metadata.handle);
		yamlStreamWriter.SetKey("type", (uint32_t)metadata.type);
		yamlStreamWriter.SetKey("version", shaderDef->GetVersion());
		//

		yamlStreamWriter.SetKey("name", shaderDef->GetName());
		yamlStreamWriter.SetKey("entryPoint", shaderDef->GetEntryPoint());
		yamlStreamWriter.SetKey("internal", shaderDef->IsInternal());

		yamlStreamWriter.BeginSequence("paths");
		for (const auto& path : shaderDef->GetSourceFiles())
		{
			yamlStreamWriter.AddValue(path);
		}
		yamlStreamWriter.EndSequence();
		yamlStreamWriter.EndMap();
	
		yamlStreamWriter.WriteToDisk();
	}

	bool ShaderDefinitionSerializer::Deserialize(const AssetMetadata& metadata, Ref<Asset> destinationAsset) const
	{
		const auto filePath = AssetManager::GetFilesystemPath(metadata.filePath);

		if (!std::filesystem::exists(filePath))
		{
			VT_CORE_ERROR("File {0} not found!", metadata.filePath);
			destinationAsset->SetFlag(AssetFlag::Missing, true);
			return false;
		}

		YAMLFileStreamReader yamlStreamReader{};

		if (!yamlStreamReader.OpenFile(filePath))
		{
			VT_CORE_ERROR("Failed to open file: {0}!", metadata.filePath);
			destinationAsset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		SerializedAssetMetadata serializedMetadata{};
		serializedMetadata.handle = yamlStreamReader.ReadAtKey("assetHandle", AssetHandle(0));
		serializedMetadata.type = (AssetType)yamlStreamReader.ReadAtKey("type", 0u);
		serializedMetadata.version = yamlStreamReader.ReadAtKey("version", 1u);

		VT_CORE_ASSERT(serializedMetadata.version == destinationAsset->GetVersion(), "Incompatible version!");

		const std::string name = yamlStreamReader.ReadAtKey("name", std::string("Unnamed"));
		const std::string entryPoint = yamlStreamReader.ReadAtKey("entryPoint", std::string("main"));
		const bool isInternal = yamlStreamReader.ReadAtKey("internal", false);

		if (!yamlStreamReader.HasKey("paths"))
		{
			VT_CORE_ERROR("No shaders defined in shader definition {0}!", metadata.filePath);
			destinationAsset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		std::vector<std::filesystem::path> paths;
		yamlStreamReader.ForEach("paths", [&]()
		{
			paths.emplace_back(yamlStreamReader.ReadValue<std::string>());
		});

		std::vector<std::string> permutationValues;
		if (yamlStreamReader.HasKey("permutations"))
		{
			yamlStreamReader.ForEach("permutations", [&]()
			{
				permutationValues.emplace_back(yamlStreamReader.ReadValue<std::string>());
			});
		}

		Ref<ShaderDefinition> shaderDef = std::reinterpret_pointer_cast<ShaderDefinition>(destinationAsset);
		shaderDef->m_isInternal = isInternal;
		shaderDef->m_name = name;
		shaderDef->m_sourceFiles = paths;
		shaderDef->m_permutaionValues = permutationValues;
		shaderDef->m_entryPoint = entryPoint;

		return true;
	}
}
