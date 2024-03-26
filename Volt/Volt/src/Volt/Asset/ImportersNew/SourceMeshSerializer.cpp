#include "vtpch.h"
#include "SourceMeshSerializer.h"

#include "Volt/Asset/AssetManager.h"
#include "Volt/Asset/Importers/MeshTypeImporter.h"

namespace Volt
{
	bool SourceMeshSerializer::Deserialize(const AssetMetadata& metadata, Ref<Asset> destinationAsset) const
	{
		const auto filePath = AssetManager::GetFilesystemPath(metadata.filePath);

		if (!std::filesystem::exists(filePath))
		{
			VT_CORE_ERROR("File {0} not found!", metadata.filePath);
			destinationAsset->SetFlag(AssetFlag::Missing, true);
			return false;
		}

		Ref<Mesh> destinationMesh = std::reinterpret_pointer_cast<Mesh>(destinationAsset);
		if (!MeshTypeImporter::ImportMesh(filePath, *destinationMesh))
		{
			destinationAsset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		return true;
	}
}
