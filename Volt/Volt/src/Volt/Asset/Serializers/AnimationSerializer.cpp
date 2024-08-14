#include "vtpch.h"
#include "AnimationSerializer.h"

#include <AssetSystem/AssetManager.h>
#include "Volt/Asset/Animation/Animation.h"

namespace Volt
{
	struct AnimationSerializationData
	{
		float duration;
		uint32_t framesPerSecond;
		Vector<Animation::Pose> frames;
		Vector<Animation::Event> events;

		static void Serialize(BinaryStreamWriter& streamWriter, const AnimationSerializationData& data)
		{
			streamWriter.Write(data.duration);
			streamWriter.Write(data.framesPerSecond);
			streamWriter.Write(data.frames);
			streamWriter.Write(data.events);
		}

		static void Deserialize(BinaryStreamReader& streamReader, AnimationSerializationData& outData)
		{
			streamReader.Read(outData.duration);
			streamReader.Read(outData.framesPerSecond);
			streamReader.Read(outData.frames);
			streamReader.Read(outData.events);
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
		serializationData.events = animation->m_events;
	
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

		AnimationSerializationData serializationData{};
		streamReader.Read(serializationData);

		Ref<Animation> animation = std::reinterpret_pointer_cast<Animation>(destinationAsset);

		animation->m_duration = serializationData.duration;
		animation->m_framesPerSecond = serializationData.framesPerSecond;
		animation->m_frames = serializationData.frames;
		animation->m_events = serializationData.events;

		return true;
	}
}
