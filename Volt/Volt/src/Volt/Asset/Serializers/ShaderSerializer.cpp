#include "vtpch.h"
#include "ShaderSerializer.h"

#include "Volt/Asset/AssetManager.h"

#include "Volt/Rendering/Shader/Shader.h"

#include "Volt/Utility/FileIO/YAMLFileStreamWriter.h"
#include "Volt/Utility/FileIO/YAMLFileStreamReader.h"

namespace Volt
{
	void ShaderSerializer::Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) const
	{
		Ref<Shader> shader = std::reinterpret_pointer_cast<Shader>(asset);
		
		const auto filePath = AssetManager::GetFilesystemPath(metadata.filePath);
		YAMLFileStreamWriter yamlStreamWriter{ filePath };
		yamlStreamWriter.BeginMap();

		// Serialize AssetMetadata
		yamlStreamWriter.SetKey("assetHandle", metadata.handle);
		yamlStreamWriter.SetKey("type", (uint32_t)metadata.type);
		yamlStreamWriter.SetKey("version", shader->GetVersion());
		//

		yamlStreamWriter.SetKey("name", shader->GetName());
		yamlStreamWriter.SetKey("internal", shader->IsInternal());

		yamlStreamWriter.BeginSequence("paths");
		for (const auto& path : shader->GetSourcePaths())
		{
			yamlStreamWriter.GetEmitter() << path;
		}
		yamlStreamWriter.EndSequence();

		yamlStreamWriter.BeginSequence("inputTextures");
		for (const auto& [shaderName, editorName] : shader->GetResources().shaderTextureDefinitions)
		{
			yamlStreamWriter.BeginMap();
			yamlStreamWriter.SetKey("shaderName", shaderName);
			yamlStreamWriter.SetKey("editorName", editorName);
			yamlStreamWriter.EndMap();
		}
		yamlStreamWriter.EndSequence();
		yamlStreamWriter.EndMap();
	
		yamlStreamWriter.WriteToDisk();
	}

	bool ShaderSerializer::Deserialize(const AssetMetadata& metadata, Ref<Asset> destinationAsset) const
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
		serializedMetadata.handle = yamlStreamReader.ReadKey("assetHandle", AssetHandle(0));
		serializedMetadata.type = (AssetType)yamlStreamReader.ReadKey("type", 0u);
		serializedMetadata.version = yamlStreamReader.ReadKey("version", 1u);

		VT_CORE_ASSERT(serializedMetadata.version == destinationAsset->GetVersion(), "Incompatible version!");

		const std::string name = yamlStreamReader.ReadKey("name", std::string("Unnamed"));
		const bool isInternal = yamlStreamReader.ReadKey("internal", false);

		std::vector<std::filesystem::path> paths;
		yamlStreamReader.ForEach("paths", [&]() 
		{
			paths.emplace_back(yamlStreamReader.GetCurrentNode().as<std::string>());
		});

		if (paths.empty())
		{
			VT_CORE_ERROR("No shaders defined in shader definition {0}!", metadata.filePath);
			destinationAsset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		std::vector<ShaderTexture> inputTextures;
		yamlStreamReader.ForEach("inputTextures", [&]() 
		{
			inputTextures.emplace_back(yamlStreamReader.ReadKey("shaderName", std::string("Empty")), yamlStreamReader.ReadKey("editorName", std::string("Null")));
		});

		Ref<Shader> shader = std::reinterpret_pointer_cast<Shader>(destinationAsset);
		shader->Initialize(name, paths, false);

		const_cast<std::vector<ShaderTexture>&>(shader->GetResources().shaderTextureDefinitions) = inputTextures; // #TODO_Ivar: Add checking
		shader->myIsInternal = isInternal;

		return true;
	}
}
