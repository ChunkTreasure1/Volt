#include "vtpch.h"
#include "BlendSpaceSerializer.h"

#include "Volt/Asset/AssetManager.h"
#include "Volt/Animation/BlendSpace.h"

namespace Volt
{
	constexpr uint32_t CURRENT_ASSET_VERSION = 1;

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

		std::vector<SerializedAnimation> animations;

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
		const size_t compressedDataOffset = AssetSerializer::WriteMetadata(metadata, CURRENT_ASSET_VERSION, streamWriter);

		streamWriter.Write(serializationData);

		const auto filePath = AssetManager::GetFilesystemPath(metadata.filePath);
		streamWriter.WriteToDisk(filePath, true, compressedDataOffset);
	}

	bool BlendSpaceSerializer::Deserialize(const AssetMetadata& metadata, Ref<Asset> destinationAsset) const
	{
		return false;
	}
}
