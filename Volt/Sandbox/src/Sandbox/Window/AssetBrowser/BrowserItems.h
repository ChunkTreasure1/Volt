#pragma once

#include "Sandbox/Utility/AssetPreview.h"

#include <Volt/Asset/Asset.h>

namespace AssetBrowser
{
	class SelectionManager;
	class Item
	{
	public:
		Item(SelectionManager* selectionManager, const std::filesystem::path& path);
		virtual ~Item() = default;
		virtual bool Render() = 0;

		std::filesystem::path path;

	protected:
		SelectionManager* mySelectionManager;
	};
}

//class DirectoryItem : public AssetBrowserItem
//{
//public:
//	~DirectoryItem() override = default;
//	void Render() override;
//
//	DirectoryItem* parentDirectory;
//
//	std::vector<Ref<AssetItem>> assets;
//	std::vector<Ref<DirectoryItem>> subDirectories;
//};