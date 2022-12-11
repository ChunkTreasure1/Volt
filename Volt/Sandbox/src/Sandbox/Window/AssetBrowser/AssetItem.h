#pragma once

#include "Sandbox/Window/AssetBrowser/BrowserItems.h"
#include "Sandbox/Utility/EditorUtilities.h"

namespace Volt
{
	class Image2D;
}

namespace AssetBrowser
{
	class AssetItem : public Item
	{
	public:
		AssetItem(SelectionManager* selectionManager, const std::filesystem::path& path, float& thumbnailSize, MeshImportData& meshImportData);
		~AssetItem() override = default;
		bool Render() override;

		Volt::AssetHandle handle = 0;
		Volt::AssetType type = Volt::AssetType::None;

		Ref<AssetPreview> preview;
		MeshImportData& meshImportData;
		
		bool isRenaming = false;
		std::string currentRenamingName;

	private:
		bool RenderRightClickPopup();

		float& myThumbnailSize;

		Ref<Volt::Image2D> GetIcon() const;
		const ImVec4 GetBackgroundColorFromType(Volt::AssetType type);
	};
}
