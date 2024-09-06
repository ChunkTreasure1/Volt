#pragma once

#include <AssetSystem/Asset.h>

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
	AssetDirectoryProcessor(Weak<AssetBrowser::SelectionManager> selectionManager, std::set<AssetType> assetMask);

	Ref<AssetBrowser::DirectoryItem> ProcessDirectories(const std::filesystem::path& path, MeshImportData& meshImportData, AssetData& meshToImportData);

private:
	Weak<AssetBrowser::SelectionManager> m_selectionManager;
	std::set<AssetType> m_assetMask;
};
