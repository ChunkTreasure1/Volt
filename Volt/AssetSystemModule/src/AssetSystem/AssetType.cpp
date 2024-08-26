#include "aspch.h"
#include "AssetType.h"

#include <CoreUtilities/FileIO/BinaryStreamWriter.h>
#include <CoreUtilities/FileIO/BinaryStreamReader.h>

AssetTypeRegistry g_assetTypeRegistry;

VT_REGISTER_ASSET_TYPE(None);

bool AssetTypeRegistry::RegisterAssetType(const VoltGUID& guid, AssetType type)
{
	// #TODO_Ivar: HACK
	if (m_typeMap.contains(guid))
	{
		return false;
	}

	VT_ENSURE(!m_typeMap.contains(guid));
	m_typeMap[guid] = type;

	return true;
}

AssetType AssetTypeRegistry::GetTypeFromGUID(const VoltGUID& guid) const
{
	VT_ENSURE(m_typeMap.contains(guid));
	return m_typeMap.at(guid);
}

AssetType AssetTypeRegistry::GetTypeFromExtension(const std::string& extension) const
{
	for (const auto& [guid, type] : m_typeMap)
	{
		const auto& extensions = type->GetExtensions();
		auto it = std::find(extensions.begin(), extensions.end(), extension);
		if (it != extensions.end())
		{
			return type;
		}
	}

	return AssetTypes::None;
}
