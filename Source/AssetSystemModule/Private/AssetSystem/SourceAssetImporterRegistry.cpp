#include "aspch.h"
#include "AssetSystem/SourceAssetImporterRegistry.h"

SourceAssetImporterRegistry g_sourceAssetImporterRegistry;

bool SourceAssetImporterRegistry::RegisterImporter(const Vector<std::string>& assignedExtensions, Ref<Volt::SourceAssetImporter> importer)
{
	for (const auto& ext : assignedExtensions)
	{
		VT_ENSURE(!m_importers.contains(ext));
		m_importers[ext] = importer;
	}

	return true;
}

Volt::SourceAssetImporter& SourceAssetImporterRegistry::GetImporterForExtension(const std::string& extension) const
{
	VT_ENSURE(m_importers.contains(extension));
	return *m_importers.at(extension);
}
