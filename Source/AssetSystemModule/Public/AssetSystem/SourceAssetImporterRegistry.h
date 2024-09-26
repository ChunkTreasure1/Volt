#pragma once

#include "AssetSystem/Config.h"

#include <CoreUtilities/Containers/Vector.h>
#include <CoreUtilities/Containers/Map.h>

namespace Volt
{
	class SourceAssetImporter;
}

class VTAS_API SourceAssetImporterRegistry
{
public:
	bool RegisterImporter(const Vector<std::string>& assignedExtensions, Ref<Volt::SourceAssetImporter> importer);
	bool ImporterForExtensionExists(const std::string& extension) const;
	Volt::SourceAssetImporter& GetImporterForExtension(const std::string& extension) const;
	

private:
	vt::map<std::string, Ref<Volt::SourceAssetImporter>> m_importers;
};

extern VTAS_API SourceAssetImporterRegistry g_sourceAssetImporterRegistry;

VT_NODISCARD VT_INLINE SourceAssetImporterRegistry& GetSourceAssetImporterRegistry()
{
	return g_sourceAssetImporterRegistry;
}

#define VT_REGISTER_SOURCE_ASSET_IMPORTER(extensions, importerClass) \
	inline static bool SourceAssetImporter_ ## importerClass ## _Registered = GetSourceAssetImporterRegistry().RegisterImporter(Vector<std::string>extensions, CreateRef<importerClass>())
