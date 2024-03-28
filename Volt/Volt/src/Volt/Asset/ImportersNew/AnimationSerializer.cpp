#include "vtpch.h"
#include "AnimationSerializer.h"

#include "Volt/Asset/AssetManager.h"
#include "Volt/Asset/Animation/Animation.h"

namespace Volt
{
	struct AnimationSerializationData
	{
		float duration;
		uint32_t framesPerSecond;
		std::vector<Animation::Pose> frames;

		static void Serialize(BinaryStreamWriter& streamWriter, const AnimationSerializationData& data)
		{
			streamWriter.Write(data.duration);
			streamWriter.Write(data.framesPerSecond);
			streamWriter.Write(data.frames);
		}

		static void Deserialize(BinaryStreamReader& streamReader, AnimationSerializationData& outData)
		{
			streamReader.Read(outData.duration);
			streamReader.Read(outData.framesPerSecond);
			streamReader.Read(outData.frames);
		}
	};

	void AnimationSerializer::Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) const
	{
		Ref<Animation> animation = std::reinterpret_pointer_cast<Animation>(asset);

		BinaryStreamWriter streamWriter{};

		AnimationSerializationData serializationData{};
		serializationData.duration = animation->m_duration;
		serializationData.framesPerSecond = animation->m_framesPerSecond;
		serializationData.frames = animation->m_frames;
	
		const size_t compressedDataOffset = AssetSerializer::WriteMetadata(metadata, asset->GetVersion(), streamWriter);
		streamWriter.Write(serializationData);

		const auto filePath = AssetManager::GetFilesystemPath(metadata.filePath);
		streamWriter.WriteToDisk(filePath, true, compressedDataOffset);
	}

	bool AnimationSerializer::Deserialize(const AssetMetadata& metadata, Ref<Asset> destinationAsset) const
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
		VT_CORE_ASSERT(serializedMetadata.version == destinationAsset->GetVersion(), "Incompatible version!")

		AnimationSerializationData serializationData{};
		streamReader.Read(serializationData);

		Ref<Animation> animation = std::reinterpret_pointer_cast<Animation>(destinationAsset);

		animation->m_duration = serializationData.duration;
		animation->m_framesPerSecond = serializationData.framesPerSecond;
		animation->m_frames = serializationData.frames;

		return true;
	}
}
