#include "vtpch.h"
#include "ShaderSerializer.h"

#include "Volt/Asset/AssetManager.h"

#include "Volt/Rendering/Shader/Shader.h"

#include "Volt/Utility/FileIO/YAMLMemoryStreamWriter.h"
#include "Volt/Utility/FileIO/YAMLMemoryStreamReader.h"

namespace Volt
{
	void ShaderSerializer::Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) const
	{
		Ref<Shader> shader = std::reinterpret_pointer_cast<Shader>(asset);
		
		YAMLMemoryStreamWriter yamlStreamWriter{};
		yamlStreamWriter.BeginMap();
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
	
		BinaryStreamWriter streamWriter{};
		const size_t compressedDataOffset = AssetSerializer::WriteMetadata(metadata, asset->GetVersion(), streamWriter);
		
		Buffer buffer = yamlStreamWriter.WriteAndGetBuffer();
		streamWriter.Write(buffer);
		buffer.Release();

		const auto filePath = AssetManager::GetFilesystemPath(metadata.filePath);
		streamWriter.WriteToDisk(filePath, true, compressedDataOffset);
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

		BinaryStreamReader streamReader{ filePath };

		if (!streamReader.IsStreamValid())
		{
			VT_CORE_ERROR("Failed to open file: {0}!", metadata.filePath);
			destinationAsset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		SerializedAssetMetadata serializedMetadata = AssetSerializer::ReadMetadata(streamReader);
		VT_CORE_ASSERT(serializedMetadata.version == destinationAsset->GetVersion(), "Incompatible version!");

		Buffer buffer{};
		streamReader.Read(buffer);

		YAMLMemoryStreamReader yamlStreamReader{};
		if (!yamlStreamReader.ConsumeBuffer(buffer))
		{
			destinationAsset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

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
