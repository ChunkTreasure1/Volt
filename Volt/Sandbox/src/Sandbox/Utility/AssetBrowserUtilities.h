#pragma once

#include <imgui.h>

class AssetBrowserUtilities
{
public:
	static const float GetBrowserItemPadding();

	static const ImVec2 GetBrowserItemSize();
	static const ImVec2 GetBrowserItemMinPos();

	static const ImVec4 GetBrowserItemHoveredColor();
	static const ImVec4 GetBrowserItemClickedColor();
	static const ImVec4 GetBrowserItemSelectedColor();
	static const ImVec4 GetBrowserItemDefaultColor();

private:
	AssetBrowserUtilities() = delete;
};