#pragma once

#include "Volt/Asset/Asset.h"

#include <imgui.h>
#include <functional>

namespace AssetBrowser
{
	class AssetItem;
	class AssetBrowserUtilities
	{
	public:
		static const float GetBrowserItemPadding();

		static const ImVec2 GetBrowserItemSize(const float thumbnailSize);
		static const ImVec2 GetBrowserItemMinPos();

		static const ImVec4 GetBrowserItemHoveredColor();
		static const ImVec4 GetBrowserItemClickedColor();
		static const ImVec4 GetBrowserItemSelectedColor();
		static const ImVec4 GetBrowserItemDefaultColor();

		static const ImVec4 GetBackgroundColor(bool isHovered, bool isSeleted);

		static bool RenderAssetTypePopup(AssetItem* item);

	private:
		AssetBrowserUtilities() = delete;

		static const std::unordered_map<Volt::AssetType, std::function<void(AssetItem*)>>& GetPopupRenderFunctions();
	};
}