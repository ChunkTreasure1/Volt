#include "vtpch.h"
#include "BlendSpaceSerializer.h"

#include <AssetSystem/AssetManager.h>
#include "Volt/Animation/BlendSpace.h"

namespace Volt
{
	struct SerializedAnimation
	{
		AssetHandle handle;
		glm::vec2 value;
	};

	struct BlendSpaceSerializationData
	{
		BlendSpaceDimension dimension;
		glm::vec2 horizontalValues;
		glm::vec2 verticalValues;

		Vector<SerializedAnimation> animations;

		static void Serialize(BinaryStreamWriter& streamWriter, const BlendSpaceSerializationData& data)
		{
			streamWriter.Write(data.dimension);
			streamWriter.Write(data.horizontalValues);
			streamWriter.Write(data.verticalValues);
			streamWriter.WriteRaw(data.animations);
		}

		static void Deserialize(BinaryStreamReader& streamReader, BlendSpaceSerializationData& outData)
		{
			streamReader.Read(outData.dimension);
			streamReader.Read(outData.horizontalValues);
			streamReader.Read(outData.verticalValues);
			streamReader.ReadRaw(outData.animations);
		}
	};

	void BlendSpaceSerializer::Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) const
	{
		Ref<BlendSpace> blendSpace = std::reinterpret_pointer_cast<BlendSpace>(asset);

		BlendSpaceSerializationData serializationData{};
		serializationData.dimension = blendSpace->myDimension;
		serializationData.horizontalValues = blendSpace->myHorizontalValues;
		serializationData.verticalValues = blendSpace->myVerticalValues;

		for (const auto& anim : blendSpace->myAnimations)
		{
			auto& serAnim = serializationData.animations.emplace_back();
			serAnim.handle = anim.second;
			serAnim.value = anim.first;
		}

		BinaryStreamWriter streamWriter{};
		const size_t compressedDataOffset = AssetSerializer::WriteMetadata(metadata, asset->GetVersion(), streamWriter);

		streamWriter.Write(serializationData);

		const auto filePath = AssetManager::GetFilesystemPath(metadata.filePath);
		streamWriter.WriteToDisk(filePath, true, compressedDataOffset);
	}

	bool BlendSpaceSerializer::Deserialize(const AssetMetadata& metadata, Ref<Asset> destinationAsset) const
	{
		const auto filePath = AssetManager::GetFilesystemPath(metadata.filePath);

		if (!std::filesystem::exists(filePath))
		{
			VT_LOG(Error, "File {0} not found!", metadata.filePath);
			destinationAsset->SetFlag(AssetFlag::Missing, true);
			return false;
		}

		BinaryStreamReader streamReader{ filePath };

		if (!streamReader.IsStreamValid())
		{
			VT_LOG(Error, "Failed to open file: {0}!", metadata.filePath);
			destinationAsset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		SerializedAssetMetadata serializedMetadata = AssetSerializer::ReadMetadata(streamReader);
		VT_ASSERT_MSG(serializedMetadata.version == destinationAsset->GetVersion(), "Incompatible version!");

		BlendSpaceSerializationData serializationData{};
		streamReader.Read(serializationData);

		Ref<BlendSpace> blendSpace = std::reinterpret_pointer_cast<BlendSpace>(destinationAsset);
		for (const auto& serAnim : serializationData.animations)
		{
			blendSpace->myAnimations.emplace_back(serAnim.value, serAnim.handle);
		}

		blendSpace->myDimension = serializationData.dimension;
		blendSpace->myHorizontalValues = serializationData.horizontalValues;
		blendSpace->myVerticalValues = serializationData.verticalValues;

		return true;
	}
}
