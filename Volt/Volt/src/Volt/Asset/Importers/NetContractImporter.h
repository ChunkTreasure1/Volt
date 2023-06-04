#pragma once
#include <Volt/Asset/Importers/AssetImporter.h>

namespace Volt
{
	class NetContractImporter : public AssetImporter
	{
		bool Load(const std::filesystem::path& path, Ref<Asset>& asset) const override;
		void Save(const Ref<Asset>& asset) const override;

		void SaveBinary(uint8_t* buffer, const Ref<Asset>& asset) const override {}
		bool LoadBinary(const uint8_t* buffer, const AssetPacker::AssetHeader& header, Ref<Asset>& asset) const override { return false; }
	};
}
