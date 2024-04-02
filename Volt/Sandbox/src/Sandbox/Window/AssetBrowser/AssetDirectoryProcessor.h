#pragma once

#include "Volt/Asset/Asset.h"

namespace AssetBrowser
{
	class SelectionManager;
	class DirectoryItem;
}

struct MeshImportData;
struct AssetData;

class AssetDirectoryProcessor
{
public:
	AssetDirectoryProcessor(Weak<AssetBrowser::SelectionManager> selectionManager, Volt::AssetType assetMask);

	Ref<AssetBrowser::DirectoryItem> ProcessDirectories(const std::filesystem::path& path, MeshImportData& meshImportData, AssetData& meshToImportData);

private:
	Weak<AssetBrowser::SelectionManager> m_selectionManager;
	Volt::AssetType m_assetMask;
};
