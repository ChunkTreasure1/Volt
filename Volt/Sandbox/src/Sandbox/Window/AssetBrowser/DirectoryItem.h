#pragma once

#include "Sandbox/Window/AssetBrowser/BrowserItems.h"

namespace AssetBrowser
{
	class AssetItem;
	class DirectoryItem : public Item
	{
	public:
		DirectoryItem(SelectionManager* selectionManager, const std::filesystem::path& path, float& thumbnailSize);
		~DirectoryItem() override = default;

		bool Render() override;
	
		DirectoryItem* parentDirectory = nullptr;

		std::vector<Ref<AssetItem>> assets;
		std::vector<Ref<DirectoryItem>> subDirectories;

		bool isNext = false;
		bool isRenaming = false;
		std::string currentRenamingName;

	private:
		bool RenderRightClickPopup();
		float& myThumbnailSize;
	};
}