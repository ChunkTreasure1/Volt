#include "vtpch.h"
#include "Volt/Asset/Importers/VTNavMeshImporter.h"
#include "Volt/Asset/Importers/VTMeshImporter.h"

#include <AssetSystem/AssetManager.h>
#include "Volt/Asset/Mesh/MeshCompiler.h"

#include "Volt/Core/Application.h"

#include <Navigation/Core/NavigationSystem.h>
#include <Navigation/Filesystem/NavMeshImporter.h>

namespace Volt
{
	bool VTNavMeshImporter::Load(const AssetMetadata& metadata, Ref<Asset>& asset) const
	{
		VT_PROFILE_FUNCTION();

		auto realPath = AssetManager::GetFilesystemPath(metadata.filePath);

		std::ifstream file(realPath, std::ios::in | std::ios::binary);
		if (!file.is_open())
		{
			VT_LOG(Error, "Could not open navmesh file!");
			return false;
		}

		auto dtnm = CreateRef<dtNavMesh>();
		if (!AI::NavMeshImporter::LoadNavMeshLegacy(file, dtnm))
		{
			return false;
		}

		file.close();

		std::reinterpret_pointer_cast<AI::NavMesh>(asset)->Initialize(dtnm);
		return true;
	}

	void VTNavMeshImporter::Save(const AssetMetadata& metadata, const Ref<Asset>& asset) const
	{
		auto navmesh = reinterpret_pointer_cast<AI::NavMesh>(asset);
		auto realPath = AssetManager::GetFilesystemPath(metadata.filePath);

		if (std::filesystem::exists(realPath))
		{
			std::filesystem::remove(realPath);
		}

		std::ofstream output(realPath, std::ios::binary | std::ios::app);
		if (!output.is_open())
		{
			VT_LOG(Error, "Could not save navmesh file!");
			return;
		}

		//AI::NavMeshImporter::SaveNavMesh(output, navmesh);

		output.close();
	}
}
