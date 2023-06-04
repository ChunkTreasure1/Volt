#pragma once

#include "AssetImporter.h"

#include <Navigation/NavMesh/VTNavMesh.h>

namespace Volt
{
	class VTNavMeshImporter : public AssetImporter
	{
	public:
		VTNavMeshImporter() = default;

		bool Load(const std::filesystem::path& path, Ref<Asset>& asset) const override;
		void Save(const Ref<Asset>& asset) const override;

		void SaveBinary(uint8_t* buffer, const Ref<Asset>& asset) const override;
		bool LoadBinary(const uint8_t* buffer, const AssetPacker::AssetHeader& header, Ref<Asset>& asset) const override;
	};
}
