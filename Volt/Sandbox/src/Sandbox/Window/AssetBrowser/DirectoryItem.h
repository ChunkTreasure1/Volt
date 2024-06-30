#pragma once

#include "Sandbox/Window/AssetBrowser/BrowserItems.h"

namespace AssetBrowser
{
	class AssetItem;
	class DirectoryItem : public Item
	{
	public:
		DirectoryItem(SelectionManager* selectionManager, const std::filesystem::path& path);
		~DirectoryItem() override = default;

		bool Render() override;

		DirectoryItem* parentDirectory = nullptr;

		std::vector<Ref<AssetItem>> assets;
		std::vector<Ref<DirectoryItem>> subDirectories;

		bool isNext = false;

	protected:
		void PushID() override;
		RefPtr<Volt::RHI::Image2D> GetIcon() const override;
		ImVec4 GetBackgroundColor() const override;
		std::string GetTypeName() const override;
		void SetDragDropPayload() override;
		bool RenderRightClickPopup() override;
		bool Rename(const std::string& newName) override;
		void Open() override;
		
		
	private:
		void RecursivlyRenameAssets(DirectoryItem* directory, const std::filesystem::path& targetDirectory);
	};
}
