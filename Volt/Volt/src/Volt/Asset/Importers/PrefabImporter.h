#pragma once

#include "Volt/Asset/Importers/AssetImporter.h"

namespace Volt
{
	class PrefabImporter : public AssetImporter
	{
	public:
		PrefabImporter();
		~PrefabImporter() override;

		bool Load(const AssetMetadata& metadata, Ref<Asset>& asset) const override;
		void Save(const AssetMetadata& metadata, const Ref<Asset>& asset) const override;

		inline static const PrefabImporter& Get() { return *s_instance; }

	private:
		inline static PrefabImporter* s_instance = nullptr;
	};
}
