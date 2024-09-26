#pragma once

#include <AssetSystem/SourceAssetImporter.h>
#include <AssetSystem/SourceAssetImporterRegistry.h>

#include <LogModule/LogCategory.h>

VT_DECLARE_LOG_CATEGORY(LogCommonTextureSourceImporter, LogVerbosity::Trace);

namespace Volt
{
	class CommonTextureSourceImporter : public SourceAssetImporter
	{
	protected:
		Vector<Ref<Asset>> ImportInternal(const std::filesystem::path& filepath, const void* config, const SourceAssetUserImportData& userData) const override;
		SourceAssetFileInformation GetSourceFileInformation(const std::filesystem::path& filepath) const override;
	};

	VT_REGISTER_SOURCE_ASSET_IMPORTER(({ ".jpeg", ".jpg", ".png", ".tga", ".bmp", ".psd", ".gif", ".hdr", ".pic", ".pnm" }), CommonTextureSourceImporter);
}
