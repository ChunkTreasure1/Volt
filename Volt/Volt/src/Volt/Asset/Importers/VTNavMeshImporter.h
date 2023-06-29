#pragma once

#include "AssetImporter.h"

#include <Navigation/NavMesh/VTNavMesh.h>

namespace Volt
{
	class VTNavMeshImporter : public AssetImporter
	{
	public:
		VTNavMeshImporter() = default;

		bool Load(const AssetMetadata& metadata, Ref<Asset>& asset) const override;
		void Save(const AssetMetadata& metadata, const Ref<Asset>& asset) const override;
	};
}
