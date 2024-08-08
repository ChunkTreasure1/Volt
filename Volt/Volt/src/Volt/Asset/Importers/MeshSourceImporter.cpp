#include "vtpch.h"
#include "MeshSourceImporter.h"

#include "MeshTypeImporter.h"

#include "Volt/Asset/Mesh/Mesh.h"
#include "Volt/Asset/AssetManager.h"

namespace Volt
{
	bool MeshSourceImporter::Load(const AssetMetadata& metadata, Ref<Asset>& asset) const
	{
		const auto filePath = AssetManager::GetFilesystemPath(metadata.filePath);

		if (!std::filesystem::exists(filePath))
		{
			VT_LOG(Error, "File {0} not found!", metadata.filePath);
			asset->SetFlag(AssetFlag::Missing, true);
			return false;
		}

		Ref<Mesh> destinationMesh = std::reinterpret_pointer_cast<Mesh>(asset);
		if (!MeshTypeImporter::ImportMesh(filePath, *destinationMesh))
		{
			asset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		return true;
	}

	void MeshSourceImporter::Save(const AssetMetadata&, const Ref<Asset>&) const
	{

	}
}
