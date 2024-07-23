#pragma once

#include "Sandbox/Utility/AssetPreview.h"

#include <Volt/Asset/Asset.h>

#include <imgui.h>

namespace AssetBrowser
{
	class SelectionManager;
	class Item
	{
	public:
		Item(SelectionManager* selectionManager, const std::filesystem::path& path);
		virtual ~Item() = default;
		virtual bool Render();
		void StartRename();

		std::filesystem::path path;

		bool isDirectory = false;

	protected:
		float GetThumbnailSize() const;

		virtual void PushID() = 0;
		virtual RefPtr<Volt::RHI::Image2D> GetIcon() const = 0;
		virtual ImVec4 GetBackgroundColor() const = 0;
		virtual std::string GetTypeName() const = 0;

		virtual void Open() {};
		virtual void SetDragDropPayload() {};
		virtual bool RenderRightClickPopup() { return false; };
		virtual bool Rename(const std::string& aNewName) { return false; };
		virtual void DrawAdditionalHoverInfo() {};

		void DrawHoverInfo(std::string_view aInfoTitle, std::string_view aInfo);

		SelectionManager* mySelectionManager;
		bool myIsRenaming;
		bool myLastRenaming;
		std::string myCurrentRenamingName;
		std::string myTypeName;

	private:
		glm::vec4 GetTypeNameColor(bool aHoverFlag,bool aSelectedFlag) const;

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
//	Vector<Ref<AssetItem>> assets;
//	Vector<Ref<DirectoryItem>> subDirectories;
//};
