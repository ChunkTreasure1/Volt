#pragma once

#include <AssetSystem/Asset.h>

namespace AssetBrowser
{
	class SelectionManager;
	class DirectoryItem;
}

struct AssetData;

class AssetDirectoryProcessor
{
public:
	AssetDirectoryProcessor(Weak<AssetBrowser::SelectionManager> selectionManager, std::set<AssetType> assetMask);

	Ref<AssetBrowser::DirectoryItem> ProcessDirectories(const std::filesystem::path& path, AssetData& meshToImportData);

private:
	Weak<AssetBrowser::SelectionManager> m_selectionManager;
	std::set<AssetType> m_assetMask;
};
