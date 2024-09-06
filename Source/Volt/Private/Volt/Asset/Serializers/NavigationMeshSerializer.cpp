#include "vtpch.h"
#include "Volt/Asset/Serializers/NavigationMeshSerializer.h"

#include <AssetSystem/AssetManager.h>

#include <Navigation/Core/NavigationSystem.h>
#include <Navigation/Filesystem/NavMeshImporter.h>

namespace Volt
{
	void NavigationMeshSerializer::Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) const
	{
		Ref<AI::NavMesh> navMesh = std::reinterpret_pointer_cast<AI::NavMesh>(asset);

		BinaryStreamWriter streamWriter{};
		const size_t compressedDataOffset = AssetSerializer::WriteMetadata(metadata, asset->GetVersion(), streamWriter);

		AI::NavMeshImporter::SaveNavMesh(streamWriter, navMesh);

		const auto filePath = AssetManager::GetFilesystemPath(metadata.filePath);
		streamWriter.WriteToDisk(filePath, true, compressedDataOffset);
	}

	bool NavigationMeshSerializer::Deserialize(const AssetMetadata& metadata, Ref<Asset> destinationAsset) const
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

		auto dtnm = CreateRef<dtNavMesh>();
		AI::NavMeshImporter::LoadNavMesh(streamReader, dtnm);

		Ref<AI::NavMesh> navMesh = std::reinterpret_pointer_cast<AI::NavMesh>(destinationAsset);
		navMesh->Initialize(dtnm);

		return true;
	}
}
