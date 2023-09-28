#pragma once

#include "Sandbox/Window/AssetBrowser/BrowserItems.h"
#include "Sandbox/Utility/EditorUtilities.h"

#include <imgui.h>

namespace Volt
{
	class Image2D;
}

namespace AssetBrowser
{
	class AssetItem : public Item
	{
	public:
		AssetItem(SelectionManager* selectionManager, const std::filesystem::path& path, MeshImportData& meshImportData, AssetData& meshToImportData);
		~AssetItem() override = default;
		bool Render() override;

		Volt::AssetHandle handle = 0;
		Volt::AssetType type = Volt::AssetType::None;

		Ref<Volt::Image2D> previewImage;

		MeshImportData& meshImportData;
		AssetData& meshToImportData;

	protected:
		void PushID() override;
		Ref<Volt::Image2D> GetIcon() const override;
		ImVec4 GetBackgroundColor() const override;
		std::string GetTypeName() const override;
		
		void SetDragDropPayload() override;
		bool RenderRightClickPopup() override;
		bool Rename(const std::string& aNewName) override;
		void Open() override;
		void DrawAdditionalHoverInfo() override;
	private:
		Volt::AssetHandle mySceneToOpen = Volt::Asset::Null();

		const ImVec4 GetBackgroundColorFromType(Volt::AssetType type) const;
	};
}
