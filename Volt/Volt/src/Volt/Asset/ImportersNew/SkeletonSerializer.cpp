#include "vtpch.h"
#include "SkeletonSerializer.h"

#include "Volt/Asset/Animation/Skeleton.h"
#include "Volt/Asset/AssetManager.h"

namespace Volt
{
	struct SkeletonSerializationData
	{
		std::string name;
		std::vector<Skeleton::Joint> joints;
		std::vector<glm::mat4> inverseBindPose;
		std::vector<Animation::TRS> restPose;

		static void Serialize(BinaryStreamWriter& streamWriter, const SkeletonSerializationData& data)
		{
			streamWriter.Write(data.name);
			streamWriter.Write(data.joints);
			streamWriter.Write(data.inverseBindPose);
			streamWriter.WriteRaw(data.restPose);
		}

		static void Deserialize(BinaryStreamReader& streamReader, SkeletonSerializationData& outData)
		{
			streamReader.Read(outData.name);
			streamReader.Read(outData.joints);
			streamReader.Read(outData.inverseBindPose);
			streamReader.ReadRaw(outData.restPose);
		}
	};

	void SkeletonSerializer::Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) const
	{
		Ref<Skeleton> skeleton = std::reinterpret_pointer_cast<Skeleton>(asset);
		BinaryStreamWriter streamWriter{};
	
		SkeletonSerializationData serializationData{};
		serializationData.name = skeleton->myName;
		serializationData.joints = skeleton->myJoints;
		serializationData.inverseBindPose = skeleton->myInverseBindPose;
		serializationData.restPose = skeleton->myRestPose;

		const size_t compressedDataOffset = AssetSerializer::WriteMetadata(metadata, asset->GetVersion(), streamWriter);
		streamWriter.Write(serializationData);

		const auto filePath = AssetManager::GetFilesystemPath(metadata.filePath);
		streamWriter.WriteToDisk(filePath, true, compressedDataOffset);
	}

	bool SkeletonSerializer::Deserialize(const AssetMetadata& metadata, Ref<Asset> destinationAsset) const
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

		SkeletonSerializationData serializationData{};
		streamReader.Read(serializationData);

		Ref<Skeleton> skeleton = std::reinterpret_pointer_cast<Skeleton>(destinationAsset);

		skeleton->myName = serializationData.name;
		skeleton->myJoints = serializationData.joints;
		skeleton->myInverseBindPose = serializationData.inverseBindPose;
		skeleton->myRestPose = serializationData.restPose;

		return true;
	}
}
