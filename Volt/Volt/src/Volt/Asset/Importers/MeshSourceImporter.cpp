#include "vtpch.h"
#include "MeshSourceImporter.h"

#include "MeshTypeImporter.h"
#include "Volt/Log/Log.h"

#include "Volt/Asset/Mesh/Mesh.h"
#include "Volt/Project/ProjectManager.h"

namespace Volt
{
	bool MeshSourceImporter::Load(const std::filesystem::path& path, Ref<Asset>& asset) const
	{
		asset = CreateRef<Mesh>();
		const auto filePath = ProjectManager::GetDirectory() / path;

		if (!std::filesystem::exists(filePath)) [[unlikely]]
		{
			VT_CORE_ERROR("File {0} not found!", path.string().c_str());
			asset->SetFlag(AssetFlag::Missing, true);
			return false;
		}
		auto mesh = MeshTypeImporter::ImportMesh(filePath);

		if (!mesh) [[unlikely]]
		{
			asset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		asset = mesh;
		asset->path = path;
		return true;
	}

	void MeshSourceImporter::Save(const Ref<Asset>&) const
	{

	}
	void MeshSourceImporter::SaveBinary(uint8_t*, const Ref<Asset>&) const
	{}
	bool MeshSourceImporter::LoadBinary(const uint8_t*, const AssetPacker::AssetHeader&, Ref<Asset>&) const
	{
		return false;
	}
}