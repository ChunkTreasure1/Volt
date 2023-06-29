#include "vtpch.h"
#include "MeshSourceImporter.h"

#include "MeshTypeImporter.h"
#include "Volt/Log/Log.h"

#include "Volt/Asset/Mesh/Mesh.h"
#include "Volt/Asset/AssetManager.h"

namespace Volt
{
	bool MeshSourceImporter::Load(const AssetMetadata& metadata, Ref<Asset>& asset) const
	{
		asset = CreateRef<Mesh>();
		const auto filePath = AssetManager::GetFilesystemPath(metadata.filePath);

		if (!std::filesystem::exists(filePath))
		{
			VT_CORE_ERROR("File {0} not found!", metadata.filePath);
			asset->SetFlag(AssetFlag::Missing, true);
			return false;
		}
		auto mesh = MeshTypeImporter::ImportMesh(filePath);

		if (!mesh)
		{
			asset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		asset = mesh;
		return true;
	}

	void MeshSourceImporter::Save(const AssetMetadata& metadata, const Ref<Asset>&) const
	{

	}
}
