#include "sbpch.h"
#include "AssetBrowserUtilities.h"

const float AssetBrowserUtilities::GetBrowserItemPadding()
{
	return 10.f;
}

const ImVec2 AssetBrowserUtilities::GetBrowserItemSize()
{
	constexpr float thumbnailSize = 85.f;
	constexpr float itemHeightModifier = 40.f;
	const float itemPadding = GetBrowserItemPadding();

	return { thumbnailSize + itemPadding, thumbnailSize + itemHeightModifier + itemPadding };
}

const ImVec2 AssetBrowserUtilities::GetBrowserItemMinPos()
{
	const ImVec2 cursorPos = ImGui::GetCursorPos();
	const ImVec2 windowPos = ImGui::GetWindowPos();
	const float scrollYOffset = ImGui::GetScrollY();

	return cursorPos + windowPos - ImVec2{ 0.f, scrollYOffset };
}

const ImVec4 AssetBrowserUtilities::GetBrowserItemHoveredColor()
{
	return { 0.2f, 0.56f, 1.f, 1.f };
}

const ImVec4 AssetBrowserUtilities::GetBrowserItemClickedColor()
{
	return { 0.3f, 0.6f, 1.f, 1.f };
}

const ImVec4 AssetBrowserUtilities::GetBrowserItemSelectedColor()
{
	return { 0.f, 0.44f, 1.f, 1.f };
}

const ImVec4 AssetBrowserUtilities::GetBrowserItemDefaultColor()
{
	return { 0.28f, 0.28f, 0.28f, 1.f };
}

