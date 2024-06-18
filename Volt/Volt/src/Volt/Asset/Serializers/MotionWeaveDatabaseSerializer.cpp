#include "vtpch.h"
#include "MotionWeaveDatabaseSerializer.h"

#include "Volt/Asset/AssetManager.h"
#include "Volt/Asset/Animation/MotionWeaveDatabase.h"

#include <format>

namespace Volt
{
	struct SerializedAnimation
	{
		AssetHandle handle;
	};

	struct MotionWeaveDatabaseSerializationData
	{
		std::vector<SerializedAnimation> animations;
		AssetHandle skeleton;

		static void Serialize(BinaryStreamWriter& streamWriter, const MotionWeaveDatabaseSerializationData& data)
		{
			streamWriter.WriteRaw(data.animations);
		}

		static void Deserialize(BinaryStreamReader& streamReader, MotionWeaveDatabaseSerializationData& outData)
		{
			streamReader.ReadRaw(outData.animations);
		}
	};

	void MotionWeaveDatabaseSerializer::Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) const
	{
		Ref<MotionWeaveDatabase> motionWeaveDatabase = std::reinterpret_pointer_cast<MotionWeaveDatabase>(asset);

		MotionWeaveDatabaseSerializationData serializationData{};
		for (const auto& anim : motionWeaveDatabase->m_AnimationHandles)
		{
			serializationData.animations.emplace_back(SerializedAnimation{ anim });
		}

		serializationData.skeleton = motionWeaveDatabase->m_Skeleton;

		BinaryStreamWriter streamWriter{};
		const size_t compressedDataOffset = AssetSerializer::WriteMetadata(metadata, asset->GetVersion(), streamWriter);

		streamWriter.Write(serializationData);

		const auto filePath = AssetManager::GetFilesystemPath(metadata.filePath);
		streamWriter.WriteToDisk(filePath, true, compressedDataOffset);
	}

	bool MotionWeaveDatabaseSerializer::Deserialize(const AssetMetadata& metadata, Ref<Asset> destinationAsset) const
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

		MotionWeaveDatabaseSerializationData serializationData{};
		streamReader.Read(serializationData);

		Ref<MotionWeaveDatabase> motionWeaveDatabase = std::reinterpret_pointer_cast<MotionWeaveDatabase>(destinationAsset);
		for (const auto& anim : serializationData.animations)
		{
			motionWeaveDatabase->m_AnimationHandles.emplace_back(anim.handle);
		}

		motionWeaveDatabase->m_Skeleton = serializationData.skeleton;

		std::string logStr = std::format("Loaded Notion Weave Database {0} with Skeleton: {1} and Animations: \n",
			(uint64_t)metadata.handle, (uint64_t)motionWeaveDatabase->m_Skeleton);

		for (const auto& anim : serializationData.animations)
		{
			logStr += std::format("		- {0}\n", (uint64_t)anim.handle);
			AssetManager::AddDependencyToAsset(metadata.handle, anim.handle);
		}

		return true;
	}
}
