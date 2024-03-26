#include "vtpch.h"
#include "AnimationGraphSerializer.h"

#include "Volt/Asset/Animation/AnimationGraphAsset.h"
#include "Volt/Asset/AssetManager.h"
#include "Volt/Utility/FileIO/YAMLMemoryStreamWriter.h"
#include "Volt/Utility/FileIO/YAMLMemoryStreamReader.h"

namespace Volt
{
	void AnimationGraphSerializer::Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) const
	{
		const Ref<AnimationGraphAsset> animGraph = std::reinterpret_pointer_cast<AnimationGraphAsset>(asset);

		YAMLMemoryStreamWriter yamlStreamWriter;
		yamlStreamWriter.BeginMap();
		yamlStreamWriter.BeginMapNamned("AnimationGraph");

		yamlStreamWriter.SetKey("state", animGraph->myGraphState);
		yamlStreamWriter.SetKey("skeleton", animGraph->mySkeletonHandle);

		GraphKey::Graph::Serialize(animGraph, yamlStreamWriter.GetEmitter());

		yamlStreamWriter.EndMap();
		yamlStreamWriter.EndMap();

		BinaryStreamWriter streamWriter{};
		const size_t compressedDataOffset = AssetSerializer::WriteMetadata(metadata, asset->GetVersion(), streamWriter);

		Buffer buffer = yamlStreamWriter.WriteAndGetBuffer();
		streamWriter.Write(buffer);
		buffer.Release();

		const auto filePath = AssetManager::GetFilesystemPath(metadata.filePath);
		streamWriter.WriteToDisk(filePath, true, compressedDataOffset);
	}

	bool AnimationGraphSerializer::Deserialize(const AssetMetadata& metadata, Ref<Asset> destinationAsset) const
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

		Buffer yamlBuffer{};
		streamReader.Read(yamlBuffer);

		YAMLMemoryStreamReader yamlStreamReader{};
		yamlStreamReader.ConsumeBuffer(yamlBuffer);

		Ref<AnimationGraphAsset> animGraph = std::reinterpret_pointer_cast<AnimationGraphAsset>(destinationAsset);
		
		yamlStreamReader.EnterScope("AnimationGraph");
		animGraph->myGraphState = yamlStreamReader.ReadKey("state", std::string(""));
		animGraph->mySkeletonHandle = yamlStreamReader.ReadKey("skeleton", AssetHandle(0));

		yamlStreamReader.EnterScope("Graph");
		GraphKey::Graph::Deserialize(animGraph, yamlStreamReader.GetCurrentNode());
		yamlStreamReader.ExitScope();

		yamlStreamReader.ExitScope();

		// Necessary to set up the skeleton correctly
		animGraph->SetSkeletonHandle(animGraph->mySkeletonHandle);

		return true;
	}
}
