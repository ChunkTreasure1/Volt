#pragma once

#include <AssetSystem/SourceAssetImporter.h>
#include <AssetSystem/SourceAssetImporterRegistry.h>

#include <LogModule/LogCategory.h>

VT_DECLARE_LOG_CATEGORY(LogDDSTextureSourceImporter, LogVerbosity::Trace);

namespace Volt
{
	class DDSTextureSourceImporter final : public SourceAssetImporter
	{
	protected:
		Vector<Ref<Asset>> ImportInternal(const std::filesystem::path& filepath, const void* config, const SourceAssetUserImportData& userData) override;
	};

	VT_REGISTER_SOURCE_ASSET_IMPORTER(({ ".dds" }), DDSTextureSourceImporter);
}
