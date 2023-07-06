#include "vtpch.h"
#include "UIUtility.h"

#include "Volt/Rendering/Texture/Texture2D.h"
#include "Volt/Rendering/Texture/Image2D.h"
#include "Volt/ImGui/ImGuiImplementation.h"

#include <backends/imgui_impl_dx11.h>
#include <backends/imgui_impl_vulkan.h>

inline static constexpr float PROPERTY_ROW_HEIGHT = 17.f;
inline static constexpr float PROPERTY_ROW_PADDING = 4.f;

ImTextureID UI::GetTextureID(Ref<Volt::Texture2D> texture)
{
	ImTextureID id = ImGui_ImplVulkan_AddTexture(texture->GetImage()->GetSampler(), texture->GetImage()->GetView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	return id;
}

ImTextureID UI::GetTextureID(Ref<Volt::Image2D> texture)
{
	ImTextureID id = ImGui_ImplVulkan_AddTexture(texture->GetSampler(), texture->GetView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	return id;
}

ImTextureID UI::GetTextureID(Volt::Texture2D* texture)
{
	ImTextureID id = ImGui_ImplVulkan_AddTexture(texture->GetImage()->GetSampler(), texture->GetImage()->GetView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	return id;
}

void UI::Header(const std::string& text)
{
	ScopedFont font{ FontType::Regular_20 };
	ImGui::TextUnformatted(text.c_str());
}

ImGuiToastType UI::ToastTypeFromNotificationType(NotificationType type)
{
	switch (type)
	{
		case NotificationType::Info: return ImGuiToastType_Info;
		case NotificationType::Warning: return ImGuiToastType_Warning;
		case NotificationType::Error: return ImGuiToastType_Error;
		case NotificationType::Success: return ImGuiToastType_Success;
		default: return ImGuiToastType_None;
	}
}

void UI::ShiftCursor(float x, float y)
{
	ImVec2 pos = { ImGui::GetCursorPosX() + x, ImGui::GetCursorPosY() + y };
	ImGui::SetCursorPos(pos);
}

bool UI::BeginPopup(const std::string& name, ImGuiWindowFlags flags)
{
	flags |= ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings;

	const uint32_t nameHash = static_cast<uint32_t>(std::hash<std::string>()(name));
	return ImGui::BeginPopupEx(nameHash, flags);
}

bool UI::BeginPopupItem(const std::string& id, ImGuiPopupFlags flags)
{
	if (id.empty())
	{
		return ImGui::BeginPopupContextItem();
	}

	return ImGui::BeginPopupContextItem(id.c_str(), flags);
}

bool UI::BeginPopupWindow(const std::string& id)
{
	if (id.empty())
	{
		return ImGui::BeginPopupContextWindow();
	}

	return ImGui::BeginPopupContextWindow(id.c_str());
}

void UI::EndPopup()
{
	ImGui::EndPopup();
}

bool UI::InputText(const std::string& name, std::string& text, ImGuiInputTextFlags_ flags)
{
	if (!name.empty())
	{
		ImGui::TextUnformatted(name.c_str());
		ImGui::SameLine();
	}

	std::string id = "##" + std::to_string(s_stackId++);
	return ImGui::InputTextString(id.c_str(), &text, flags);
}

void UI::PushFont(FontType font)
{
	ImGui::PushFont(s_fonts.at(font));
}

void UI::PopFont()
{
	ImGui::PopFont();
}

int32_t UI::LevenshteinDistance(const std::string& str1, const std::string& str2)
{
	int32_t m = (int32_t)str1.length();
	int32_t n = (int32_t)str2.length();
	std::vector<std::vector<int>> dp(m + 1, std::vector<int>(n + 1));

	// Initializing the first row and column as 0
	for (int i = 0; i <= m; i++)
	{
		dp[i][0] = i;
	}
	for (int j = 0; j <= n; j++)
	{
		dp[0][j] = j;
	}

	// Filling in the rest of the dp array
	for (int i = 1; i <= m; i++)
	{
		for (int j = 1; j <= n; j++)
		{
			int insertion = dp[i][j - 1] + 1;
			int deletion = dp[i - 1][j] + 1;
			int match = dp[i - 1][j - 1];
			int mismatch = dp[i - 1][j - 1] + 1;
			if (str1[i - 1] == str2[j - 1])
			{
				dp[i][j] = std::min(std::min(insertion, deletion), match);
			}
			else
			{
				dp[i][j] = std::min(std::min(insertion, deletion), mismatch);
			}
		}
	}
	return dp[m][n];
}

const std::vector<std::string> UI::GetEntriesMatchingQuery(const std::string& query, const std::vector<std::string>& entries)
{
	std::multimap<int32_t, std::string> scores{};

	for (const auto& entry : entries)
	{
		const int32_t score = LevenshteinDistance(query, entry);
		scores.emplace(score, entry);
	}

	std::vector<std::string> result{};
	for (const auto& [score, entry] : scores)
	{
		if (!Utils::StringContains(Utils::ToLower(entry), Utils::ToLower(query)))
		{
			continue;
		}

		result.emplace_back(entry);
	}

	return result;
}

void UI::RenderMatchingTextBackground(const std::string& query, const std::string& text, const glm::vec4& color, const glm::uvec2& offset)
{
	const auto matchOffset = Utils::ToLower(text).find(Utils::ToLower(query));

	if (matchOffset == std::string::npos)
	{
		return;
	}

	const auto matchPrefix = text.substr(0, matchOffset);
	const auto match = text.substr(matchOffset, query.size());

	const auto prefixSize = ImGui::CalcTextSize(matchPrefix.c_str());
	const auto matchSize = ImGui::CalcTextSize(match.c_str());
	const auto cursorPos = ImGui::GetCursorPos();
	const auto windowPos = ImGui::GetWindowPos();
	const auto scrollX = ImGui::GetScrollX();
	const auto scrollY = ImGui::GetScrollY();

	auto currentWindow = ImGui::GetCurrentWindow();
	const ImColor imguiCol = ImColor(color.x, color.y, color.z, color.w);

	const ImVec2 min = { cursorPos.x - scrollX + prefixSize.x + offset.x, cursorPos.y - scrollY + offset.y };
	const ImVec2 max = { cursorPos.x - scrollX + prefixSize.x + matchSize.x + offset.x, cursorPos.y + matchSize.y - scrollY + offset.y };

	currentWindow->DrawList->AddRectFilled(min + windowPos, max + windowPos, imguiCol);
}

void UI::RenderHighlightedBackground(const glm::vec4& color, float height)
{
	auto currentWindow = ImGui::GetCurrentWindow();
	const auto windowPos = ImGui::GetWindowPos();
	const auto availRegion = ImGui::GetContentRegionMax();
	const auto cursorPos = ImGui::GetCursorPos();

	const auto scrollX = ImGui::GetScrollX();
	const auto scrollY = ImGui::GetScrollY();

	const ImVec2 min = ImGui::GetWindowPos() + ImVec2{ -scrollX, cursorPos.y - scrollY };
	const ImVec2 max = ImGui::GetWindowPos() + ImVec2{ availRegion.x - scrollX, height + cursorPos.y - scrollY };
	currentWindow->DrawList->AddRectFilled(min, max, ImColor{ color.x, color.y, color.z, color.w });
}

bool UI::IsPropertyRowHovered()
{
	const ImVec2 rowAreaMin = ImGui::TableGetCellBgRect(ImGui::GetCurrentTable(), 0).Min;
	const ImVec2 rowAreaMax = { ImGui::TableGetCellBgRect(ImGui::GetCurrentTable(), ImGui::TableGetColumnCount() - 1).Max.x, rowAreaMin.y + PROPERTY_ROW_HEIGHT + PROPERTY_ROW_PADDING * 2.f };

	ImGui::PushClipRect(rowAreaMin, rowAreaMax, false);
	const bool isRowHovered = ImGui::IsMouseHoveringRect(rowAreaMin, rowAreaMax, true);
	ImGui::PopClipRect();

	return isRowHovered;
}

void UI::SetPropertyBackgroundColor()
{
	static const glm::vec4 PropertyBackground = { 36.f / 255.f, 36.f / 255.f, 36.f / 255.f, 1.f };
	static const glm::vec4 PropertyBackgroundHovered = { 47.f / 255.f, 47.f / 255.f, 47.f / 255.f, 1.f };

	if (IsPropertyRowHovered())
	{
		SetRowColor(PropertyBackgroundHovered);
	}
	else
	{
		SetRowColor(PropertyBackground);
	}
}

void UI::SetRowColor(const glm::vec4& color)
{
	for (int32_t i = 0; i < ImGui::TableGetColumnCount(); i++)
	{
		ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImColor{ color.x, color.y, color.z, color.w }, i);
	}
}

void UI::BeginPropertyRow()
{
	auto* window = ImGui::GetCurrentWindow();
	window->DC.CurrLineSize.y = PROPERTY_ROW_HEIGHT;

	ImGui::TableNextRow(0, PROPERTY_ROW_HEIGHT);
	ImGui::TableNextColumn();
	window->DC.CurrLineTextBaseOffset = 3.f;

	SetPropertyBackgroundColor();
}

bool UI::InputTextWithHint(const std::string& name, std::string& text, const std::string& hint, ImGuiInputTextFlags_ flags /* = ImGuiInputTextFlags_None */)
{
	if (!name.empty())
	{
		ImGui::TextUnformatted(name.c_str());
		ImGui::SameLine();
	}

	std::string id = "##" + std::to_string(s_stackId++);
	return ImGui::InputTextWithHintString(id.c_str(), hint.c_str(), &text, flags);
}

bool UI::InputTextMultiline(const std::string& name, std::string& text, ImGuiInputTextFlags_ flags)
{
	if (!name.empty())
	{
		ImGui::TextUnformatted(name.c_str());
		ImGui::SameLine();
	}

	std::string id = "##" + std::to_string(s_stackId++);
	return ImGui::InputTextMultilineString(id.c_str(), &text, ImVec2{ 0.f, 0.f }, flags);
}

bool UI::ImageButton(const std::string& id, ImTextureID textureId, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, int frame_padding, const ImVec4& bg_col, const ImVec4& tint_col)
{
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;
	if (window->SkipItems)
		return false;

	const ImGuiID imId = window->GetID(id.c_str());

	// Default to using texture ID as ID. User can still push string/integer prefixes.
	const ImVec2 padding = (frame_padding >= 0) ? ImVec2((float)frame_padding, (float)frame_padding) : g.Style.FramePadding;
	return ImGui::ImageButtonEx(imId, textureId, size, uv0, uv1, padding, bg_col, tint_col);
}

bool UI::ImageButtonState(const std::string& id, bool state, ImTextureID textureId, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1)
{
	if (state)
	{
		return ImageButton(id, textureId, size, uv0, uv1, -1, { 0.18f, 0.18f, 0.18f, 1.f });
	}
	else
	{
		return ImageButton(id, textureId, size, uv0, uv1);
	}
}

bool UI::TreeNodeImage(Ref<Volt::Texture2D> texture, const std::string& text, ImGuiTreeNodeFlags flags, bool setOpen)
{
	ScopedStyleFloat2 frame{ ImGuiStyleVar_FramePadding, { 0.f, 0.f } };
	ScopedStyleFloat2 spacing{ ImGuiStyleVar_ItemSpacing, { 0.f, 0.f } };

	const ImVec2 size = ImGui::CalcTextSize(text.c_str());

	ImGui::Image(GetTextureID(texture), { size.y, size.y });
	ImGui::SameLine();

	if (setOpen)
	{
		ImGui::SetNextItemOpen(true, ImGuiCond_Always);
	}

	return ImGui::TreeNodeEx(text.c_str(), flags);
}

bool UI::TreeNodeFramed(const std::string& text, bool alwaysOpen, float rounding)
{
	ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_Framed |
		ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding;

	if (alwaysOpen)
	{
		nodeFlags |= ImGuiTreeNodeFlags_DefaultOpen;
	}

	UI::ScopedStyleFloat frameRound(ImGuiStyleVar_FrameRounding, rounding);

	return ImGui::TreeNodeEx(text.c_str(), nodeFlags);
}

bool UI::TreeNodeWidth(const std::string& text, float width, float rounding, ImGuiTreeNodeFlags flags)
{
	const ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding | flags;
	UI::ScopedStyleFloat frameRound(ImGuiStyleVar_FrameRounding, rounding);

	return ImGui::TreeNodeWidthEx(text.c_str(), width, nodeFlags);
}

void UI::SameLine(float offsetX, float spacing)
{
	ImGui::SameLine(offsetX, spacing);
}

bool UI::ImageSelectable(Ref<Volt::Texture2D> texture, const std::string& text, bool& selected)
{
	const ImVec2 size = ImGui::CalcTextSize(text.c_str());

	ImGui::Image(GetTextureID(texture), { size.y, size.y });
	ImGui::SameLine();

	return ImGui::Selectable(text.c_str(), &selected);
}

bool UI::ImageSelectable(Ref<Volt::Texture2D> texture, const std::string& text)
{
	const ImVec2 size = ImGui::CalcTextSize(text.c_str());

	ImGui::Image(GetTextureID(texture), { size.y - 2.f, size.y - 2.f });
	ImGui::SameLine();

	UI::ShiftCursor(5.f, 0.f);

	return ImGui::Selectable(text.c_str());
}

bool UI::ImageSelectable(Ref<Volt::Texture2D> texture, const std::string& text, bool selected)
{
	ImVec2 size = ImGui::CalcTextSize(text.c_str());
	ImGui::Image(GetTextureID(texture), { size.y, size.y }, { 0, 1 }, { 1, 0 });
	ImGui::SameLine();
	return ImGui::Selectable(text.c_str(), selected, ImGuiSelectableFlags_SpanAvailWidth);
}

void UI::PopID()
{
	ImGui::PopID();
	s_contextId--;
}

void UI::PushID()
{
	int id = s_contextId++;
	ImGui::PushID(id);
	s_stackId = 0;
}

int32_t UI::GetID()
{
	return s_stackId++;
}

bool UI::IsInputEnabled()
{
	const auto& io = ImGui::GetIO();
	return (io.ConfigFlags & ImGuiConfigFlags_NoMouse) == 0 && (io.ConfigFlags & ImGuiConfigFlags_NavNoCaptureKeyboard) == 0;
}

void UI::SetInputEnabled(bool enable)
{
	auto& io = ImGui::GetIO();

	if (enable)
	{
		io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
		io.ConfigFlags &= ~ImGuiConfigFlags_NavNoCaptureKeyboard;
	}
	else
	{
		io.ConfigFlags |= ImGuiConfigFlags_NoMouse;
		io.ConfigFlags |= ImGuiConfigFlags_NavNoCaptureKeyboard;
	}
}

void UI::SimpleToolTip(const std::string& toolTip)
{
	if (!toolTip.empty())
	{
		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip("%s", toolTip.c_str());
		}
	}
}

bool UI::BeginMenuBar(ImRect barRect)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	IM_ASSERT(!window->DC.MenuBarAppending);
	ImGui::BeginGroup(); // Backup position on layer 0 // FIXME: Misleading to use a group for that backup/restore
	ImGui::PushID("##menubar");

	const ImVec2 padding = window->WindowPadding;

	barRect.Min.y += padding.y;
	barRect.Max.y += padding.y;

	// We don't clip with current window clipping rectangle as it is already set to the area below. However we clip with window full rect.
	// We remove 1 worth of rounding to Max.x to that text in long menus and small windows don't tend to display over the lower-right rounded area, which looks particularly glitchy.
	ImRect bar_rect = barRect;
	ImRect clip_rect(IM_ROUND(ImMax(window->Pos.x, bar_rect.Min.x + window->WindowBorderSize + window->Pos.x)), IM_ROUND(bar_rect.Min.y + window->WindowBorderSize + window->Pos.y),
		IM_ROUND(ImMax(bar_rect.Min.x + window->Pos.x, bar_rect.Max.x - ImMax(window->WindowRounding, window->WindowBorderSize) + window->Pos.x)), IM_ROUND(bar_rect.Max.y + window->Pos.y));
	clip_rect.ClipWith(window->OuterRectClipped);
	ImGui::PushClipRect(clip_rect.Min, clip_rect.Max, false);

	// We overwrite CursorMaxPos because BeginGroup sets it to CursorPos (essentially the .EmitItem hack in EndMenuBar() would need something analoguous here, maybe a BeginGroupEx() with flags).
	window->DC.CursorPos = window->DC.CursorMaxPos = ImVec2(bar_rect.Min.x + window->Pos.x, bar_rect.Min.y + window->Pos.y);
	window->DC.LayoutType = ImGuiLayoutType_Horizontal;
	window->DC.NavLayerCurrent = ImGuiNavLayer_Menu;
	window->DC.MenuBarAppending = true;
	ImGui::AlignTextToFramePadding();
	return true;
}

void UI::EndMenuBar()
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return;
	ImGuiContext& g = *GImGui;

	// Nav: When a move request within one of our child menu failed, capture the request to navigate among our siblings.
	if (ImGui::NavMoveRequestButNoResultYet() && (g.NavMoveDir == ImGuiDir_Left || g.NavMoveDir == ImGuiDir_Right) && (g.NavWindow->Flags & ImGuiWindowFlags_ChildMenu))
	{
		// Try to find out if the request is for one of our child menu
		ImGuiWindow* nav_earliest_child = g.NavWindow;
		while (nav_earliest_child->ParentWindow && (nav_earliest_child->ParentWindow->Flags & ImGuiWindowFlags_ChildMenu))
			nav_earliest_child = nav_earliest_child->ParentWindow;
		if (nav_earliest_child->ParentWindow == window && nav_earliest_child->DC.ParentLayoutType == ImGuiLayoutType_Horizontal && (g.NavMoveFlags & ImGuiNavMoveFlags_Forwarded) == 0)
		{
			// To do so we claim focus back, restore NavId and then process the movement request for yet another frame.
			// This involve a one-frame delay which isn't very problematic in this situation. We could remove it by scoring in advance for multiple window (probably not worth bothering)
			const ImGuiNavLayer layer = ImGuiNavLayer_Menu;
			IM_ASSERT(window->DC.NavLayersActiveMaskNext & (1 << layer)); // Sanity check
			ImGui::FocusWindow(window);
			ImGui::SetNavID(window->NavLastIds[layer], layer, 0, window->NavRectRel[layer]);
			g.NavDisableHighlight = true; // Hide highlight for the current frame so we don't see the intermediary selection.
			g.NavDisableMouseHover = g.NavMousePosDirty = true;
			ImGui::NavMoveRequestForward(g.NavMoveDir, g.NavMoveClipDir, g.NavMoveFlags, g.NavMoveScrollFlags); // Repeat
		}
	}

	IM_MSVC_WARNING_SUPPRESS(6011); // Static Analysis false positive "warning C6011: Dereferencing NULL pointer 'window'"
	// IM_ASSERT(window->Flags & ImGuiWindowFlags_MenuBar); // NOTE(Yan): Needs to be commented out because Jay
	IM_ASSERT(window->DC.MenuBarAppending);
	ImGui::PopClipRect();
	ImGui::PopID();
	window->DC.MenuBarOffset.x = window->DC.CursorPos.x - window->Pos.x; // Save horizontal position so next append can reuse it. This is kinda equivalent to a per-layer CursorPos.
	g.GroupStack.back().EmitItem = false;
	ImGui::EndGroup(); // Restore position on layer 0
	window->DC.LayoutType = ImGuiLayoutType_Vertical;
	window->DC.NavLayerCurrent = ImGuiNavLayer_Main;
	window->DC.MenuBarAppending = false;
}

bool UI::BeginProperties(const std::string& name, const ImVec2 size)
{
	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 2.f);
	ImGui::PushStyleColor(ImGuiCol_Border, { 49.f / 255.f, 49.f / 255.f, 49.f / 255.f, 1.f });

	ImGui::PushStyleColor(ImGuiCol_FrameBg, { 15.f / 255.f, 15.f / 255.f, 15.f / 255.f, 1.f });
	ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, { 15.f / 255.f, 15.f / 255.f, 15.f / 255.f, 1.f });
	ImGui::PushStyleColor(ImGuiCol_FrameBgActive, { 15.f / 255.f, 15.f / 255.f, 15.f / 255.f, 1.f });

	ImGui::PushStyleColor(ImGuiCol_Separator, { 26.f / 255.f, 26.f / 255.f, 26.f / 255.f, 1.f });
	ImGui::PushStyleColor(ImGuiCol_SeparatorHovered, { 26.f / 255.f, 26.f / 255.f, 26.f / 255.f, 1.f });
	ImGui::PushStyleColor(ImGuiCol_SeparatorActive, { 26.f / 255.f, 26.f / 255.f, 26.f / 255.f, 1.f });

	bool open = ImGui::BeginTable(name.c_str(), 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_Resizable, size);

	if (open)
	{
		ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch, 0.3f);
		ImGui::TableSetupColumn("Properties", ImGuiTableColumnFlags_WidthStretch);
	}

	return open;
}

void UI::EndProperties()
{
	ImGui::EndTable();
	ImGui::PopStyleColor(7);
	ImGui::PopStyleVar(2);
}

bool UI::ComboProperty(const std::string& text, int& currentItem, const std::vector<const char*>& items, float width)
{
	bool changed = false;

	BeginPropertyRow();

	ImGui::TextUnformatted(text.c_str());

	ImGui::TableNextColumn();

	if (width == 0.f)
	{
		ImGui::PushItemWidth(ImGui::GetColumnWidth());
	}
	else
	{
		ImGui::PushItemWidth(width);
	}
	std::string id = "##" + std::to_string(s_stackId++);
	if (ImGui::Combo(id.c_str(), &currentItem, items.data(), (int32_t)items.size()))
	{
		changed = true;
	}

	ImGui::PopItemWidth();

	return changed;
}

bool UI::Combo(const std::string& text, int& currentItem, const std::vector<const char*>& items, float width)
{
	bool changed = false;

	ImGui::TextUnformatted(text.c_str());

	ImGui::SameLine();

	std::string id = "##" + std::to_string(s_stackId++);

	ImGui::SetNextItemWidth(width);
	if (ImGui::Combo(id.c_str(), &currentItem, items.data(), (int32_t)items.size()))
	{
		changed = true;
	}

	return changed;
}

bool UI::Combo(const std::string& text, int& currentItem, const std::vector<std::string>& strItems, float width)
{
	bool changed = false;

	ImGui::TextUnformatted(text.c_str());

	ImGui::SameLine();

	std::string id = "##" + std::to_string(s_stackId++);

	std::vector<const char*> items;
	std::for_each(strItems.begin(), strItems.end(), [&](const std::string& string) { items.emplace_back(string.c_str()); });

	ImGui::SetNextItemWidth(width);
	if (ImGui::Combo(id.c_str(), &currentItem, items.data(), (int32_t)items.size()))
	{
		changed = true;
	}

	return changed;
}

void* UI::DragDropTarget(const std::string& type)
{
	void* data = nullptr;

	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* pPayload = ImGui::AcceptDragDropPayload(type.c_str()))
		{
			data = pPayload->Data;
		}

		ImGui::EndDragDropTarget();
	}

	return data;
}

void* UI::DragDropTarget(std::initializer_list<std::string> types, ImGuiDragDropFlags flags)
{
	void* data = nullptr;

	for (const auto& type : types)
	{
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* pPayload = ImGui::AcceptDragDropPayload(type.c_str(), flags))
			{
				data = pPayload->Data;
			}

			ImGui::EndDragDropTarget();
		}
	}

	return data;
}

void UI::Notify(NotificationType type, const std::string& title, const std::string& content, int32_t duration)
{
	ImGuiToast toast{ ToastTypeFromNotificationType(type), duration };
	toast.set_title(title.c_str());
	toast.set_content(content.c_str());

	ImGui::InsertNotification(toast);
}

void UI::OpenModal(const std::string& name, ImGuiPopupFlags flags)
{
	const uint32_t nameHash = static_cast<uint32_t>(std::hash<std::string>()(name));
	ImGui::OpenPopupEx(nameHash, flags);
}

void UI::OpenPopup(const std::string& name, ImGuiPopupFlags flags)
{
	const uint32_t nameHash = static_cast<uint32_t>(std::hash<std::string>()(name));
	ImGui::OpenPopupEx(nameHash, flags);
}

bool UI::BeginModal(const std::string& name, ImGuiWindowFlags flags)
{
	const uint32_t nameHash = static_cast<uint32_t>(std::hash<std::string>()(name));
	return ImGui::BeginPopupModal(name.c_str(), nameHash, nullptr, flags);
}

void UI::EndModal()
{
	ImGui::EndPopup();
}

void UI::SmallSeparatorHeader(const std::string& text, float padding)
{
	UI::ScopedFont font{ FontType::Bold_16 };

	const auto pos = ImGui::GetCursorPos();
	ImGui::TextUnformatted(text.c_str());
	const auto textSize = ImGui::CalcTextSize(text.c_str());

	const auto availWidth = ImGui::GetWindowWidth();
	const auto windowPos = ImGui::GetWindowPos();

	ImGui::GetCurrentWindow()->DrawList->AddLine(pos + windowPos + ImVec2{ textSize.x + padding, textSize.y / 2.f }, { pos.x + availWidth + windowPos.x, pos.y + 1.f + windowPos.y + textSize.y / 2.f }, IM_COL32(255, 255, 255, 255));
}

bool UI::Combo(const std::string& text, int& currentItem, const char** items, uint32_t count)
{
	bool changed = false;

	ImGui::TextUnformatted(text.c_str());

	ImGui::SameLine();

	std::string id = "##" + std::to_string(s_stackId++);

	if (ImGui::Combo(id.c_str(), &currentItem, items, count))
	{
		changed = true;
	}

	return changed;
}

bool UI::ComboProperty(const std::string& text, int& currentItem, const std::vector<std::string>& strItems, float width)
{
	bool changed = false;

	BeginPropertyRow();

	ImGui::TextUnformatted(text.c_str());

	ImGui::TableNextColumn();

	if (width == 0.f)
	{
		ImGui::PushItemWidth(ImGui::GetColumnWidth());
	}
	else
	{
		ImGui::PushItemWidth(width);
	}
	std::string id = "##" + std::to_string(s_stackId++);

	std::vector<const char*> items;
	std::for_each(strItems.begin(), strItems.end(), [&](const std::string& string) { items.emplace_back(string.c_str()); });

	if (ImGui::Combo(id.c_str(), &currentItem, items.data(), (int32_t)items.size()))
	{
		changed = true;
	}

	ImGui::PopItemWidth();

	return changed;
}

void UI::TreeNodePop()
{
	ImGui::TreePop();
}

bool UI::CollapsingHeader(const std::string& label, ImGuiTreeNodeFlags flags)
{
	return ImGui::CollapsingHeader(label.c_str(), flags);
}

bool UI::ImageButton(const std::string& id, ImTextureID textureId, const ImVec2& size, const ImVec4& bg_col, const ImVec4& tint_col)
{
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;
	if (window->SkipItems)
		return false;

	const ImGuiID imId = window->GetID(id.c_str());

	const ImVec2& uv0 = ImVec2(0, 0);
	const ImVec2& uv1 = ImVec2(1, 1);
	int frame_padding = -1;

	// Default to using texture ID as ID. User can still push string/integer prefixes.
	const ImVec2 padding = (frame_padding >= 0) ? ImVec2((float)frame_padding, (float)frame_padding) : g.Style.FramePadding;
	return ImGui::ImageButtonEx(imId, textureId, size, uv0, uv1, padding, bg_col, tint_col);
}

void UI::PropertyInfoString(const std::string& key, const std::string& info)
{
	BeginPropertyRow();

	ImGui::TextUnformatted(key.c_str());

	ImGui::TableNextColumn();
	ImGui::TextUnformatted(info.c_str());
}

bool UI::PropertyAxisColor(const std::string& text, glm::vec3& value, float resetValue, std::function<void(glm::vec3& value)> callback)
{
	ScopedStyleFloat2 cellPad(ImGuiStyleVar_CellPadding, { 4.f, 0.f });

	bool changed = false;

	ImGui::TableNextRow();
	ImGui::TableNextColumn();
	SetPropertyBackgroundColor();

	ImGui::Text(text.c_str());

	ImGui::TableNextColumn();
	ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0.f, 0.f });

	float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.f;
	ImVec2 buttonSize = { lineHeight + 3.f, lineHeight };

	{
		ScopedColor color{ ImGuiCol_Button, { 0.8f, 0.1f, 0.15f, 1.f } };
		ScopedColor colorh{ ImGuiCol_ButtonHovered, { 0.9f, 0.2f, 0.2f, 1.f } };
		ScopedColor colora{ ImGuiCol_ButtonActive, { 0.8f, 0.1f, 0.15f, 1.f } };

		std::string butId = "X##" + std::to_string(s_stackId++);
		if (ImGui::Button(butId.c_str(), buttonSize))
		{
			value.x = resetValue;
			changed = true;
			if (callback)
			{
				callback(value);
			}
		}
	}

	ImGui::SameLine();
	std::string id = "##" + std::to_string(s_stackId++);

	if (ImGui::DragFloat(id.c_str(), &value.x, 0.1f))
	{
		changed = true;
		if (callback)
		{
			callback(value);
		}
	}

	if (ImGui::IsItemHovered())
	{
		if (Volt::Input::IsMouseButtonDown(VT_MOUSE_BUTTON_LEFT))
		{
			changed = true;
		}
	}

	ImGui::PopItemWidth();
	ImGui::SameLine();

	{
		ScopedColor color{ ImGuiCol_Button, { 0.2f, 0.7f, 0.2f, 1.f } };
		ScopedColor colorh{ ImGuiCol_ButtonHovered, { 0.3f, 0.8f, 0.3f, 1.f } };
		ScopedColor colora{ ImGuiCol_ButtonActive, { 0.2f, 0.7f, 0.2f, 1.f } };

		std::string butId = "Y##" + std::to_string(s_stackId++);
		if (ImGui::Button(butId.c_str(), buttonSize))
		{
			value.y = resetValue;
			changed = true;
			if (callback)
			{
				callback(value);
			}
		}
	}

	ImGui::SameLine();
	id = "##" + std::to_string(s_stackId++);

	if (ImGui::DragFloat(id.c_str(), &value.y, 0.1f))
	{
		changed = true;
		if (callback)
		{
			callback(value);
		}
	}

	if (ImGui::IsItemHovered())
	{
		if (Volt::Input::IsMouseButtonDown(VT_MOUSE_BUTTON_LEFT))
		{
			changed = true;
		}
	}

	ImGui::PopItemWidth();
	ImGui::SameLine();

	{
		ScopedColor color{ ImGuiCol_Button, { 0.1f, 0.25f, 0.8f, 1.f } };
		ScopedColor colorh{ ImGuiCol_ButtonHovered, { 0.2f, 0.35f, 0.9f, 1.f } };
		ScopedColor colora{ ImGuiCol_ButtonActive, { 0.1f, 0.25f, 0.8f, 1.f } };

		std::string butId = "Z##" + std::to_string(s_stackId++);
		if (ImGui::Button(butId.c_str(), buttonSize))
		{
			value.z = resetValue;
			changed = true;
			if (callback)
			{
				callback(value);
			}
		}
	}

	ImGui::SameLine();
	id = "##" + std::to_string(s_stackId++);
	if (ImGui::DragFloat(id.c_str(), &value.z, 0.1f))
	{
		changed = true;
		if (callback)
		{
			callback(value);
		}
	}

	if (ImGui::IsItemHovered())
	{
		if (Volt::Input::IsMouseButtonDown(VT_MOUSE_BUTTON_LEFT))
		{
			changed = true;
		}
	}

	ImGui::PopItemWidth();
	ImGui::PopStyleVar();

	return changed;
}

bool UI::PropertyAxisColor(const std::string& text, glm::vec2& value, float resetValue)
{
	ScopedStyleFloat2 cellPad(ImGuiStyleVar_CellPadding, { 4.f, 0.f });

	bool changed = false;

	BeginPropertyRow();

	ImGui::Text(text.c_str());

	ImGui::TableNextColumn();
	ImGui::PushMultiItemsWidths(2, ImGui::CalcItemWidth());

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0.f, 0.f });

	float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.f;
	ImVec2 buttonSize = { lineHeight + 3.f, lineHeight };

	{
		ScopedColor color{ ImGuiCol_Button, { 0.8f, 0.1f, 0.15f, 1.f } };
		ScopedColor colorh{ ImGuiCol_ButtonHovered, { 0.9f, 0.2f, 0.2f, 1.f } };
		ScopedColor colora{ ImGuiCol_ButtonActive, { 0.8f, 0.1f, 0.15f, 1.f } };

		std::string butId = "X##" + std::to_string(s_stackId++);
		if (ImGui::Button(butId.c_str(), buttonSize))
		{
			value.x = resetValue;
			changed = true;
		}
	}

	ImGui::SameLine();
	std::string id = "##" + std::to_string(s_stackId++);

	if (ImGui::DragFloat(id.c_str(), &value.x, 0.1f))
	{
		changed = true;
	}

	if (ImGui::IsItemHovered())
	{
		if (Volt::Input::IsMouseButtonDown(VT_MOUSE_BUTTON_LEFT))
		{
			changed = true;
		}
	}

	ImGui::PopItemWidth();
	ImGui::SameLine();

	{
		ScopedColor color{ ImGuiCol_Button, { 0.2f, 0.7f, 0.2f, 1.f } };
		ScopedColor colorh{ ImGuiCol_ButtonHovered, { 0.3f, 0.8f, 0.3f, 1.f } };
		ScopedColor colora{ ImGuiCol_ButtonActive, { 0.2f, 0.7f, 0.2f, 1.f } };

		std::string butId = "Y##" + std::to_string(s_stackId++);
		if (ImGui::Button(butId.c_str(), buttonSize))
		{
			value.y = resetValue;
			changed = true;
		}
	}

	ImGui::SameLine();
	id = "##" + std::to_string(s_stackId++);

	if (ImGui::DragFloat(id.c_str(), &value.y, 0.1f))
	{
		changed = true;
	}

	if (ImGui::IsItemHovered())
	{
		if (Volt::Input::IsMouseButtonDown(VT_MOUSE_BUTTON_LEFT))
		{
			changed = true;
		}
	}

	ImGui::PopItemWidth();
	ImGui::PopStyleVar();

	return changed;
}

bool UI::Property(const std::string& text, bool& value, std::function<void(bool& value)> callback, const std::string& toolTip)
{
	bool changed = false;

	BeginPropertyRow();

	ImGui::TextUnformatted(text.c_str());
	SimpleToolTip(toolTip);

	ImGui::TableNextColumn();
	std::string id = "##" + std::to_string(s_stackId++);

	if (ImGui::Checkbox(id.c_str(), &value))
	{
		changed = true;
		if (callback)
		{
			callback(value);
		}
	}

	return changed;
}

bool UI::Property(const std::string& text, int32_t& value, std::function<void(int32_t& value)> callback, const std::string& toolTip)
{
	bool changed = false;

	BeginPropertyRow();

	ImGui::TextUnformatted(text.c_str());
	SimpleToolTip(toolTip);

	ImGui::TableNextColumn();
	std::string id = "##" + std::to_string(s_stackId++);
	ImGui::PushItemWidth(ImGui::GetColumnWidth());

	if (ImGui::DragScalar(id.c_str(), ImGuiDataType_S32, (void*)&value, 1.f))
	{
		changed = true;
		if (callback)
		{
			callback(value);
		}
	}

	ImGui::PopItemWidth();

	return changed;
}

bool UI::Property(const std::string& text, uint32_t& value, std::function<void(uint32_t& value)> callback, const std::string& toolTip)
{
	bool changed = false;

	BeginPropertyRow();

	ImGui::TextUnformatted(text.c_str());
	SimpleToolTip(toolTip);

	ImGui::TableNextColumn();
	std::string id = "##" + std::to_string(s_stackId++);
	ImGui::PushItemWidth(ImGui::GetColumnWidth());

	if (ImGui::DragScalar(id.c_str(), ImGuiDataType_U32, (void*)&value, 1.f))
	{
		changed = true;
		if (callback)
		{
			callback(value);
		}
	}

	ImGui::PopItemWidth();

	return changed;
}

bool UI::Property(const std::string& text, int16_t& value, std::function<void(int16_t& value)> callback, const std::string& toolTip)
{
	bool changed = false;

	BeginPropertyRow();

	ImGui::TextUnformatted(text.c_str());
	SimpleToolTip(toolTip);

	ImGui::TableNextColumn();
	std::string id = "##" + std::to_string(s_stackId++);
	ImGui::PushItemWidth(ImGui::GetColumnWidth());

	if (ImGui::DragScalar(id.c_str(), ImGuiDataType_S16, (void*)&value, 1.f))
	{
		changed = true;
		if (callback)
		{
			callback(value);
		}
	}

	ImGui::PopItemWidth();

	return changed;
}

bool UI::Property(const std::string& text, uint16_t& value, std::function<void(uint16_t& value)> callback, const std::string& toolTip)
{
	bool changed = false;

	BeginPropertyRow();

	ImGui::TextUnformatted(text.c_str());
	SimpleToolTip(toolTip);

	ImGui::TableNextColumn();
	std::string id = "##" + std::to_string(s_stackId++);
	ImGui::PushItemWidth(ImGui::GetColumnWidth());

	if (ImGui::DragScalar(id.c_str(), ImGuiDataType_U16, (void*)&value, 1.f))
	{
		changed = true;
		if (callback)
		{
			callback(value);
		}
	}

	ImGui::PopItemWidth();

	return changed;
}

bool UI::Property(const std::string& text, int8_t& value, std::function<void(int8_t& value)> callback, const std::string& toolTip)
{
	bool changed = false;

	BeginPropertyRow();

	ImGui::TextUnformatted(text.c_str());
	SimpleToolTip(toolTip);

	ImGui::TableNextColumn();
	std::string id = "##" + std::to_string(s_stackId++);
	ImGui::PushItemWidth(ImGui::GetColumnWidth());

	if (ImGui::DragScalar(id.c_str(), ImGuiDataType_S8, (void*)&value, 1.f))
	{
		changed = true;
		if (callback)
		{
			callback(value);
		}
	}

	ImGui::PopItemWidth();

	return changed;
}

bool UI::Property(const std::string& text, uint8_t& value, std::function<void(uint8_t& value)> callback, const std::string& toolTip)
{
	bool changed = false;

	BeginPropertyRow();

	ImGui::TextUnformatted(text.c_str());
	SimpleToolTip(toolTip);

	ImGui::TableNextColumn();
	std::string id = "##" + std::to_string(s_stackId++);
	ImGui::PushItemWidth(ImGui::GetColumnWidth());

	if (ImGui::DragScalar(id.c_str(), ImGuiDataType_U8, (void*)&value, 1.f))
	{
		changed = true;
		if (callback)
		{
			callback(value);
		}
	}

	ImGui::PopItemWidth();

	return changed;
}

bool UI::Property(const std::string& text, double& value, std::function<void(double& value)> callback, const std::string& toolTip)
{
	bool changed = false;

	BeginPropertyRow();

	ImGui::TextUnformatted(text.c_str());
	SimpleToolTip(toolTip);

	ImGui::TableNextColumn();
	std::string id = "##" + std::to_string(s_stackId++);
	ImGui::PushItemWidth(ImGui::GetColumnWidth());

	if (ImGui::DragScalar(id.c_str(), ImGuiDataType_Double, (void*)&value, 1.f))
	{
		changed = true;
		if (callback)
		{
			callback(value);
		}
	}

	ImGui::PopItemWidth();

	return changed;
}

bool UI::Property(const std::string& text, float& value, bool useMinMax, float min, float max, std::function<void(float& value)> callback, const std::string& toolTip)
{
	bool changed = false;

	BeginPropertyRow();

	ImGui::TextUnformatted(text.c_str());
	SimpleToolTip(toolTip);

	ImGui::TableNextColumn();
	std::string id = "##" + std::to_string(s_stackId++);
	ImGui::PushItemWidth(ImGui::GetColumnWidth());

	if (useMinMax)
	{
		changed = ImGui::SliderFloat(id.c_str(), &value, min, max);
	}
	else
	{
		changed = ImGui::DragFloat(id.c_str(), &value, 1.f, min, max);
	}

	if (changed)
	{
		if (value < min && useMinMax)
		{
			value = min;
		}

		if (value > max && useMinMax)
		{
			value = max;
		}

		changed = true;
		if (callback)
		{
			callback(value);
		}
	}

	ImGui::PopItemWidth();

	return changed;
}

bool UI::Property(const std::string& text, glm::vec2& value, float min, float max, std::function<void(glm::vec2& value)> callback, const std::string& toolTip)
{
	bool changed = false;

	BeginPropertyRow();

	ImGui::TextUnformatted(text.c_str());
	SimpleToolTip(toolTip);

	ImGui::TableNextColumn();
	std::string id = "##" + std::to_string(s_stackId++);
	ImGui::PushItemWidth(ImGui::GetColumnWidth());

	if (ImGui::DragFloat2(id.c_str(), glm::value_ptr(value), 1.f, min, max))
	{
		changed = true;
		if (callback)
		{
			callback(value);
		}
	}

	ImGui::PopItemWidth();

	return changed;
}

bool UI::Property(const std::string& text, glm::vec3& value, float min, float max, std::function<void(glm::vec3& value)> callback, const std::string& toolTip)
{
	bool changed = false;

	BeginPropertyRow();

	ImGui::TextUnformatted(text.c_str());
	SimpleToolTip(toolTip);

	ImGui::TableNextColumn();
	std::string id = "##" + std::to_string(s_stackId++);
	ImGui::PushItemWidth(ImGui::GetColumnWidth());

	if (ImGui::DragFloat3(id.c_str(), glm::value_ptr(value), 1.f, min, max))
	{
		changed = true;
		if (callback)
		{
			callback(value);
		}
	}

	ImGui::PopItemWidth();

	return changed;
}

bool UI::Property(const std::string& text, glm::vec4& value, float min, float max, std::function<void(glm::vec4& value)> callback, const std::string& toolTip)
{
	bool changed = false;

	BeginPropertyRow();

	ImGui::TextUnformatted(text.c_str());
	SimpleToolTip(toolTip);

	ImGui::TableNextColumn();
	std::string id = "##" + std::to_string(s_stackId++);
	ImGui::PushItemWidth(ImGui::GetColumnWidth());

	if (ImGui::DragFloat4(id.c_str(), glm::value_ptr(value), 1.f, min, max))
	{
		changed = true;
		if (callback)
		{
			callback(value);
		}
	}

	ImGui::PopItemWidth();

	return changed;
}

bool UI::Property(const std::string& text, glm::uvec2& value, uint32_t min, uint32_t max, std::function<void(glm::uvec2& value)> callback, const std::string& toolTip)
{
	bool changed = false;

	BeginPropertyRow();

	ImGui::TextUnformatted(text.c_str());
	SimpleToolTip(toolTip);

	ImGui::TableNextColumn();
	std::string id = "##" + std::to_string(s_stackId++);
	ImGui::PushItemWidth(ImGui::GetColumnWidth());

	if (ImGui::DragScalarN(id.c_str(), ImGuiDataType_U32, glm::value_ptr(value), 2, 1.f, &min, &max))
	{
		changed = true;
		if (callback)
		{
			callback(value);
		}
	}

	ImGui::PopItemWidth();

	return changed;
}

bool UI::Property(const std::string& text, glm::uvec3& value, uint32_t min, uint32_t max, std::function<void(glm::uvec3& value)> callback, const std::string& toolTip)
{
	bool changed = false;

	BeginPropertyRow();

	ImGui::TextUnformatted(text.c_str());
	SimpleToolTip(toolTip);

	ImGui::TableNextColumn();
	std::string id = "##" + std::to_string(s_stackId++);
	ImGui::PushItemWidth(ImGui::GetColumnWidth());

	if (ImGui::DragScalarN(id.c_str(), ImGuiDataType_U32, glm::value_ptr(value), 3, 1.f, &min, &max))
	{
		changed = true;
		if (callback)
		{
			callback(value);
		}
	}

	ImGui::PopItemWidth();

	return changed;
}

bool UI::Property(const std::string& text, glm::uvec4& value, uint32_t min, uint32_t max, std::function<void(glm::uvec4& value)> callback, const std::string& toolTip)
{
	bool changed = false;

	BeginPropertyRow();

	ImGui::TextUnformatted(text.c_str());
	SimpleToolTip(toolTip);

	ImGui::TableNextColumn();
	std::string id = "##" + std::to_string(s_stackId++);
	ImGui::PushItemWidth(ImGui::GetColumnWidth());

	if (ImGui::DragScalarN(id.c_str(), ImGuiDataType_U32, glm::value_ptr(value), 4, 1.f, &min, &max))
	{
		changed = true;
		if (callback)
		{
			callback(value);
		}
	}

	ImGui::PopItemWidth();

	return changed;
}

bool UI::Property(const std::string& text, glm::ivec2& value, uint32_t min, uint32_t max, std::function<void(glm::ivec2& value)> callback, const std::string& toolTip)
{
	bool changed = false;

	BeginPropertyRow();

	ImGui::TextUnformatted(text.c_str());
	SimpleToolTip(toolTip);

	ImGui::TableNextColumn();
	std::string id = "##" + std::to_string(s_stackId++);
	ImGui::PushItemWidth(ImGui::GetColumnWidth());

	if (ImGui::DragScalarN(id.c_str(), ImGuiDataType_S32, glm::value_ptr(value), 2, 1.f, &min, &max))
	{
		changed = true;
		if (callback)
		{
			callback(value);
		}
	}

	ImGui::PopItemWidth();

	return changed;
}

bool UI::Property(const std::string& text, glm::ivec3& value, uint32_t min, uint32_t max, std::function<void(glm::ivec3& value)> callback, const std::string& toolTip)
{
	bool changed = false;

	BeginPropertyRow();

	ImGui::TextUnformatted(text.c_str());
	SimpleToolTip(toolTip);

	ImGui::TableNextColumn();
	std::string id = "##" + std::to_string(s_stackId++);
	ImGui::PushItemWidth(ImGui::GetColumnWidth());

	if (ImGui::DragScalarN(id.c_str(), ImGuiDataType_S32, glm::value_ptr(value), 3, 1.f, &min, &max))
	{
		changed = true;
		if (callback)
		{
			callback(value);
		}
	}

	ImGui::PopItemWidth();

	return changed;
}

bool UI::Property(const std::string& text, glm::ivec4& value, uint32_t min, uint32_t max, std::function<void(glm::ivec4& value)> callback, const std::string& toolTip)
{
	bool changed = false;

	BeginPropertyRow();

	ImGui::TextUnformatted(text.c_str());
	SimpleToolTip(toolTip);

	ImGui::TableNextColumn();
	std::string id = "##" + std::to_string(s_stackId++);
	ImGui::PushItemWidth(ImGui::GetColumnWidth());

	if (ImGui::DragScalarN(id.c_str(), ImGuiDataType_S32, glm::value_ptr(value), 4, 1.f, &min, &max))
	{
		changed = true;
		if (callback)
		{
			callback(value);
		}
	}

	ImGui::PopItemWidth();

	return changed;
}

bool UI::PropertyDragFloat(const std::string& text, float& value, float increment, bool useMinMax, float min, float max, std::function<void(float& value)> callback, const std::string& toolTip)
{
	bool changed = false;

	BeginPropertyRow();

	ImGui::TextUnformatted(text.c_str());
	SimpleToolTip(toolTip);

	ImGui::TableNextColumn();
	std::string id = "##" + std::to_string(s_stackId++);
	ImGui::PushItemWidth(ImGui::GetColumnWidth());

	if (ImGui::DragFloat(id.c_str(), &value, increment, min, max))
	{
		if (value < min && useMinMax)
		{
			value = min;
		}

		if (value > max && useMinMax)
		{
			value = max;
		}

		changed = true;
		if (callback)
		{
			callback(value);
		}
	}

	ImGui::PopItemWidth();

	return changed;
}

bool UI::PropertyTextBox(const std::string& text, const std::string& value, bool readOnly, const std::string& toolTip)
{
	bool changed = false;

	BeginPropertyRow();

	ImGui::TextUnformatted(text.c_str());
	SimpleToolTip(toolTip);

	ImGui::TableNextColumn();
	ImGui::PushItemWidth(ImGui::GetColumnWidth());

	if (InputText("", const_cast<std::string&>(value), readOnly ? ImGuiInputTextFlags_ReadOnly : ImGuiInputTextFlags_None))
	{
		changed = true;
	}

	ImGui::PopItemWidth();

	return changed;
}

bool UI::PropertyEntity(const std::string& text, Ref<Volt::Scene> scene, Wire::EntityId& value, std::function<void(Wire::EntityId& value)> callback, const std::string& toolTip)
{
	bool changed = false;

	BeginPropertyRow();

	ImGui::TextUnformatted(text.c_str());
	SimpleToolTip(toolTip);

	ImGui::TableNextColumn();
	std::string id = "##" + std::to_string(s_stackId++);
	ImGui::PushItemWidth(ImGui::GetColumnWidth());

	Volt::Entity entity{ value, scene.get() };

	std::string entityName;
	if (entity)
	{
		entityName = entity.GetComponent<Volt::TagComponent>().tag;
	}
	else
	{
		entityName = "Null";
	}

	ImGui::InputTextString(id.c_str(), &entityName, ImGuiInputTextFlags_ReadOnly);

	if (auto ptr = UI::DragDropTarget("scene_entity_hierarchy"))
	{
		Wire::EntityId entityId = *(Wire::EntityId*)ptr;
		value = entityId;
		changed = true;
		if (callback)
		{
			callback(value);
		}
	}

	ImGui::PopItemWidth();
	return changed;
}

bool UI::PropertyEntity(Ref<Volt::Scene> scene, Wire::EntityId& value, const float width, std::function<void(Wire::EntityId& value)> callback, const std::string& toolTip)
{
	bool changed = false;

	SimpleToolTip(toolTip);

	std::string id = "##" + std::to_string(s_stackId++);
	ImGui::PushItemWidth(width);

	Volt::Entity entity{ value, scene.get() };

	std::string entityName;
	if (entity)
	{
		entityName = entity.GetComponent<Volt::TagComponent>().tag;
	}
	else
	{
		entityName = "Null";
	}

	ImGui::InputTextString(id.c_str(), &entityName, ImGuiInputTextFlags_ReadOnly);

	if (auto ptr = UI::DragDropTarget("scene_entity_hierarchy"))
	{
		Wire::EntityId entityId = *(Wire::EntityId*)ptr;
		value = entityId;
		changed = true;
		if (callback)
		{
			callback(value);
		}
	}

	ImGui::PopItemWidth();
	return changed;
}

bool UI::Property(const std::string& text, const std::string& value, bool readOnly, std::function<void(const std::string& value)> callback, const std::string& toolTip)
{
	bool changed = false;

	BeginPropertyRow();

	ImGui::TextUnformatted(text.c_str());
	SimpleToolTip(toolTip);

	ImGui::TableNextColumn();
	ImGui::PushItemWidth(ImGui::GetColumnWidth());

	if (InputText("", const_cast<std::string&>(value), readOnly ? ImGuiInputTextFlags_ReadOnly : ImGuiInputTextFlags_None))
	{
		changed = true;
		if (callback)
		{
			callback(value);
		}
	}

	ImGui::PopItemWidth();

	return changed;
}

bool UI::Property(const std::string& text, std::string& value, bool readOnly, std::function<void(std::string& value)> callback, const std::string& toolTip)
{
	bool changed = false;

	BeginPropertyRow();

	ImGui::TextUnformatted(text.c_str());
	SimpleToolTip(toolTip);

	ImGui::TableNextColumn();
	ImGui::PushItemWidth(ImGui::GetColumnWidth());

	if (InputText("", value, readOnly ? ImGuiInputTextFlags_ReadOnly : ImGuiInputTextFlags_None))
	{
		changed = true;
		if (callback)
		{
			callback(value);
		}
	}

	return changed;
}

bool UI::PropertyColor(const std::string& text, glm::vec4& value, std::function<void(glm::vec4& value)> callback, const std::string& toolTip)
{
	BeginPropertyRow();

	ImGui::TextUnformatted(text.c_str());
	SimpleToolTip(toolTip);

	ImGui::TableNextColumn();
	std::string id = "##" + std::to_string(s_stackId++);
	ImGui::PushItemWidth(ImGui::GetColumnWidth());

	if (ImGui::ColorEdit4(id.c_str(), glm::value_ptr(value)))
	{
		if (callback)
		{
			callback(value);
		}
		return true;
	}

	return false;
}

bool UI::PropertyColor(const std::string& text, glm::vec3& value, std::function<void(glm::vec3& value)> callback, const std::string& toolTip)
{
	BeginPropertyRow();

	ImGui::TextUnformatted(text.c_str());
	SimpleToolTip(toolTip);

	ImGui::TableNextColumn();
	std::string id = "##" + std::to_string(s_stackId++);
	ImGui::PushItemWidth(ImGui::GetColumnWidth());

	if (ImGui::ColorEdit3(id.c_str(), glm::value_ptr(value)))
	{
		if (callback)
		{
			callback(value);
		}
		return true;
	}

	return false;
}

bool UI::Property(const std::string& text, std::filesystem::path& path, std::function<void(std::filesystem::path& value)> callback, const std::string& toolTip)
{
	bool changed = false;

	BeginPropertyRow();
	
	ImGui::TextUnformatted(text.c_str());
	SimpleToolTip(toolTip);

	ImGui::TableNextColumn();
	std::string sPath = path.string();
	ImGui::PushItemWidth(ImGui::GetColumnWidth() - ImGui::CalcTextSize("Open...").x - 20.f);

	if (InputText("", sPath))
	{
		path = std::filesystem::path(sPath);
		changed = true;
		if (callback)
		{
			callback(path);
		}
	}

	ImGui::PopItemWidth();
	ImGui::SameLine();

	std::string buttonId = "Open...##" + std::to_string(s_stackId++);
	if (ImGui::Button(buttonId.c_str(), { ImGui::GetContentRegionAvail().x, 25.f }))
	{
		auto newPath = FileSystem::OpenFileDialogue({ { "All (*.*)" }, { "*" } });
		if (!newPath.empty())
		{
			path = newPath;
			changed = true;
			if (callback)
			{
				callback(path);
			}
		}
	}

	return changed;
}
bool UI::PropertyDirectory(const std::string& text, std::filesystem::path& path, std::function<void(std::filesystem::path& value)> callback, const std::string& toolTip)
{
	bool changed = false;

	BeginPropertyRow();

	ImGui::TextUnformatted(text.c_str());
	SimpleToolTip(toolTip);

	ImGui::TableNextColumn();
	std::string sPath = path.string();
	ImGui::PushItemWidth(ImGui::GetColumnWidth() - ImGui::CalcTextSize("Open...").x - 20.f);

	if (InputText("", sPath))
	{
		path = std::filesystem::path(sPath);
		changed = true;
		if (callback)
		{
			callback(path);
		}
	}

	ImGui::PopItemWidth();
	ImGui::SameLine();

	std::string buttonId = "Open...##" + std::to_string(s_stackId++);
	if (ImGui::Button(buttonId.c_str(), { ImGui::GetContentRegionAvail().x, 25.f }))
	{
		auto newPath = FileSystem::PickFolderDialogue();
		if (!newPath.empty())
		{
			path = newPath;
			changed = true;
			if (callback)
			{
				callback(path);
			}
		}
	}

	return changed;
}
bool UI::PropertyMultiline(const std::string& text, std::string& value, bool readOnly, const std::string& toolTip)
{
	bool changed = false;

	BeginPropertyRow();

	ImGui::TextUnformatted(text.c_str());
	SimpleToolTip(toolTip);

	ImGui::TableNextColumn();
	ImGui::PushItemWidth(ImGui::GetColumnWidth());

	if (InputTextMultiline("", value, readOnly ? ImGuiInputTextFlags_ReadOnly : ImGuiInputTextFlags_None))
	{
		changed = true;
	}

	return changed;
}

bool UI::PropertyPassword(const std::string& text, std::string& value, bool readOnly, std::function<void(std::string& value)> callback, const std::string& toolTip)
{
	bool changed = false;

	BeginPropertyRow();

	ImGui::TextUnformatted(text.c_str());
	SimpleToolTip(toolTip);

	ImGui::TableNextColumn();
	ImGui::PushItemWidth(ImGui::GetColumnWidth());

	if (InputText("", value, readOnly ? ImGuiInputTextFlags_ReadOnly : ImGuiInputTextFlags_Password))
	{
		changed = true;
		if (callback)
		{
			callback(value);
		}
	}

	return changed;
}
