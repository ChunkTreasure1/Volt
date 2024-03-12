#include "vtpch.h"
#include "PostProcessingStackSerializer.h"

#include "Volt/Asset/AssetManager.h"
#include "Volt/Asset/Rendering/PostProcessingStack.h"

namespace Volt
{
	constexpr uint32_t CURRENT_ASSET_VERSION = 1;

	struct PostProcessingStackSerializationData
	{
		std::vector<AssetHandle> effects;
	
		static void Serialize(BinaryStreamWriter& streamWriter, const PostProcessingStackSerializationData& data)
		{
			streamWriter.WriteRaw(data.effects);
		}

		static void Deserialize(BinaryStreamReader& streamReader, PostProcessingStackSerializationData& outData)
		{
			streamReader.ReadRaw(outData.effects);
		}
	};

	void PostProcessingStackSerializer::Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) const
	{
		Ref<PostProcessingStack> postStack = std::reinterpret_pointer_cast<PostProcessingStack>(asset);

		PostProcessingStackSerializationData serializationData{};
		for (const auto& effect : postStack->myPostProcessingStack)
		{
			serializationData.effects.push_back(effect.materialHandle);
		}

		BinaryStreamWriter streamWriter{};
		const size_t compressedDataOffset = AssetSerializer::WriteMetadata(metadata, CURRENT_ASSET_VERSION, streamWriter);

		streamWriter.Write(serializationData);

		const auto filePath = AssetManager::GetFilesystemPath(metadata.filePath);
		streamWriter.WriteToDisk(filePath, true, compressedDataOffset);
	}

	bool PostProcessingStackSerializer::Deserialize(const AssetMetadata& metadata, Ref<Asset> destinationAsset) const
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

		Ref<PostProcessingStack> postStack = std::reinterpret_pointer_cast<PostProcessingStack>(destinationAsset);

		PostProcessingStackSerializationData serializationData{};
		streamReader.Read(serializationData);

		for (const auto& effectNode : serializationData.effects)
		{
			postStack->myPostProcessingStack.emplace_back(effectNode);
		}

		return true;
	}
}
