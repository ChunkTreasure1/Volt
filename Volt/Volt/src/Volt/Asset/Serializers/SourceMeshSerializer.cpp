#include "vtpch.h"
#include "SourceMeshSerializer.h"

#include "Volt/Asset/AssetManager.h"
#include "Volt/Asset/Mesh/MeshSource.h"
#include "Volt/Asset/Importers/MeshTypeImporter.h"

namespace Volt
{
	bool SourceMeshSerializer::Deserialize(const AssetMetadata& metadata, Ref<Asset> destinationAsset) const
	{
		const auto filePath = AssetManager::GetFilesystemPath(metadata.filePath);

		if (!std::filesystem::exists(filePath))
		{
			VT_LOG(LogSeverity::Error, "File {0} not found!", metadata.filePath);
			destinationAsset->SetFlag(AssetFlag::Missing, true);
			return false;
		}

		Ref<MeshSource> destinationMesh = std::reinterpret_pointer_cast<MeshSource>(destinationAsset);
		if (!MeshTypeImporter::ImportMesh(filePath, *destinationMesh->m_underlyingMesh))
		{
			destinationAsset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		return true;
	}
}
