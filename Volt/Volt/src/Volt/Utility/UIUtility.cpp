#include "vtpch.h"
#include "UIUtility.h"

#include "Volt/Rendering/Texture/Texture2D.h"

#include <RHIModule/ImGui/ImGuiImplementation.h>

inline static constexpr float PROPERTY_ROW_HEIGHT = 17.f;
inline static constexpr float PROPERTY_ROW_PADDING = 4.f;

inline static glm::vec4 ToNormalizedRGB(float r, float g, float b, float a = 255.f)
{
	return { r / 255.f, g / 255.f, b / 255.f, a / 255.f };
}

ImTextureID UI::GetTextureID(Ref<Volt::Texture2D> texture)
{
	return Volt::RHI::ImGuiImplementation::Get().GetTextureID(texture->GetImage());
}

ImTextureID UI::GetTextureID(RefPtr<Volt::RHI::Image2D> texture)
{
	return Volt::RHI::ImGuiImplementation::Get().GetTextureID(texture);
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
	Vector<Vector<int>> dp(m + 1, Vector<int>(n + 1));

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

const Vector<std::string> UI::GetEntriesMatchingQuery(const std::string& query, const Vector<std::string>& entries)
{
	std::multimap<int32_t, std::string> scores{};

	for (const auto& entry : entries)
	{
		const int32_t score = LevenshteinDistance(query, entry);
		scores.emplace(score, entry);
	}

	Vector<std::string> result{};
	for (const auto& [score, entry] : scores)
	{
		if (!Utility::StringContains(Utility::ToLower(entry), Utility::ToLower(query)))
		{
			continue;
		}

		result.emplace_back(entry);
	}

	return result;
}

void UI::RenderMatchingTextBackground(const std::string& query, const std::string& text, const glm::vec4& color, const glm::uvec2& offset)
{
	const auto matchOffset = Utility::ToLower(text).find(Utility::ToLower(query));

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

bool UI::IsPropertyColumnHovered(const uint32_t column)
{
	const ImVec2 rowAreaMin = ImGui::TableGetCellBgRect(ImGui::GetCurrentTable(), static_cast<int32_t>(column)).Min;
	const ImVec2 rowAreaMax = { ImGui::TableGetCellBgRect(ImGui::GetCurrentTable(), static_cast<int32_t>(column)).Max.x, rowAreaMin.y + PROPERTY_ROW_HEIGHT + PROPERTY_ROW_PADDING * 2.f };

	ImGui::PushClipRect(rowAreaMin, rowAreaMax, false);
	const bool isColumnHovered = ImGui::IsMouseHoveringRect(rowAreaMin, rowAreaMax, true);
	ImGui::PopClipRect();

	return isColumnHovered;
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

void UI::EndPropertyRow()
{
}

bool UI::IsItemHovered(const float itemWidth)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();

	const auto& style = ImGui::GetStyle();

	const ImVec2 label_size = ImGui::CalcTextSize("TEST", NULL, true);
	const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(itemWidth, label_size.y + style.FramePadding.y * 2.0f));

	if (ImGui::IsMouseHoveringRect(frame_bb.Min, frame_bb.Max))
	{
		return true;
	}

	return false;
}

bool UI::DragScalarN(const std::string& id, ImGuiDataType dataType, void* data, int32_t components, float speed, const void* min, const void* max)
{
	static constexpr ImGuiDataTypeInfo GDataTypeInfo[] =
	{
		{ sizeof(char),             "S8",   "%d",   "%d"    },  // ImGuiDataType_S8
		{ sizeof(unsigned char),    "U8",   "%u",   "%u"    },
		{ sizeof(short),            "S16",  "%d",   "%d"    },  // ImGuiDataType_S16
		{ sizeof(unsigned short),   "U16",  "%u",   "%u"    },
		{ sizeof(int),              "S32",  "%d",   "%d"    },  // ImGuiDataType_S32
		{ sizeof(unsigned int),     "U32",  "%u",   "%u"    },
	#ifdef _MSC_VER
		{ sizeof(ImS64),            "S64",  "%I64d","%I64d" },  // ImGuiDataType_S64
		{ sizeof(ImU64),            "U64",  "%I64u","%I64u" },
	#else
		{ sizeof(ImS64),            "S64",  "%lld", "%lld"  },  // ImGuiDataType_S64
		{ sizeof(ImU64),            "U64",  "%llu", "%llu"  },
	#endif
		{ sizeof(float),            "float", "%.3f","%f"    },  // ImGuiDataType_Float (float are promoted to double in va_arg)
		{ sizeof(double),           "double","%f",  "%lf"   },  // ImGuiDataType_Double
	};

	bool changed = false;

	ImGui::BeginGroup();
	ImGui::PushID(id.c_str());
	const float width = (ImGui::GetColumnWidth() / components) - ImGui::GetStyle().ItemInnerSpacing.x;
	size_t type_size = GDataTypeInfo[dataType].Size;

	for (int32_t i = 0; i < components; i++)
	{
		ImGui::PushID(i);

		if (i > 0)
		{
			ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
		}

		changed |= DrawItem(width, [&]()
		{
			return ImGui::DragScalar("", dataType, data, speed, min, max);
		});

		ImGui::PopID();
		data = (void*)((char*)data + type_size);
	}

	ImGui::PopID();

	ImGui::EndGroup();
	return changed;
}

bool UI::DrawItem(std::function<bool()> itemFunc)
{
	const float itemWidth = ImGui::GetColumnWidth();

	ImGui::PushItemWidth(itemWidth);

	const bool itemHovered = IsItemHovered(itemWidth);

	if (itemHovered)
	{
		static const glm::vec4 PropertyItemHovered = { 1.f };
		ImGui::PushStyleColor(ImGuiCol_Border, PropertyItemHovered);
	}

	bool changed = itemFunc();

	if (itemHovered)
	{
		ImGui::PopStyleColor();
	}

	ImGui::PopItemWidth();

	return changed;
}

bool UI::DrawItem(const float itemWidth, std::function<bool()> itemFunc)
{
	ImGui::PushItemWidth(itemWidth);

	const bool itemHovered = IsItemHovered(itemWidth);

	if (itemHovered)
	{
		static const glm::vec4 PropertyItemHovered = { 1.f };
		ImGui::PushStyleColor(ImGuiCol_Border, PropertyItemHovered);
	}

	bool changed = itemFunc();

	if (itemHovered)
	{
		ImGui::PopStyleColor();
	}

	ImGui::PopItemWidth();

	return changed;
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

bool UI::BeginListView(const std::string& strId)
{
	const glm::vec4 BACKGROUND = ToNormalizedRGB(26.f, 26.f, 26.f);

	ImGui::PushStyleColor(ImGuiCol_ChildBg, BACKGROUND);

	bool open = ImGui::BeginChild(strId.c_str(), ImGui::GetContentRegionAvail());
	return open;
}

void UI::EndListView()
{
	ImGui::PopStyleColor();
}

bool UI::BeginProperties(const std::string& name, const ImVec2 size)
{
	bool open = ImGui::BeginTable(name.c_str(), 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_Resizable, size);

	if (open)
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

bool UI::ComboProperty(const std::string& text, int& currentItem, const Vector<const char*>& items, float width)
{
	bool changed = false;

	BeginPropertyRow();

	ImGui::TextUnformatted(text.c_str());
	ImGui::TableNextColumn();

	std::string id = "##" + std::to_string(s_stackId++);
	changed = DrawItem((width == 0.f) ? ImGui::GetColumnWidth() : width, [&]()
	{
		return ImGui::Combo(id.c_str(), &currentItem, items.data(), (int32_t)items.size());
	});

	EndPropertyRow();

	return changed;
}

bool UI::Combo(const std::string& text, int& currentItem, const Vector<const char*>& items, float width)
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

bool UI::Combo(const std::string& text, int& currentItem, const Vector<std::string>& strItems, float width)
{
	bool changed = false;

	ImGui::TextUnformatted(text.c_str());

	ImGui::SameLine();

	std::string id = "##" + std::to_string(s_stackId++);

	Vector<const char*> items;
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

bool UI::ComboProperty(const std::string& text, int& currentItem, const Vector<std::string>& strItems, float width)
{
	bool changed = false;

	BeginPropertyRow();

	ImGui::TextUnformatted(text.c_str());
	ImGui::TableNextColumn();

	std::string id = "##" + std::to_string(s_stackId++);

	Vector<const char*> items;
	std::for_each(strItems.begin(), strItems.end(), [&](const std::string& string) { items.emplace_back(string.c_str()); });

	changed = DrawItem((width == 0.f) ? ImGui::GetColumnWidth() : width, [&]() 
	{
		return ImGui::Combo(id.c_str(), &currentItem, items.data(), (int32_t)items.size());
	});

	EndPropertyRow();

	return changed;
}

void UI::TreeNodePop()
{
	ImGui::TreePop();
}

bool UI::CollapsingHeader(std::string_view label, ImGuiTreeNodeFlags flags)
{
	return ImGui::CollapsingHeader(label.data(), flags);
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

	EndPropertyRow();
}

bool UI::PropertyAxisColor(const std::string& text, glm::vec3& value, float resetValue)
{
	ScopedStyleFloat2 cellPad(ImGuiStyleVar_CellPadding, { 4.f, 0.f });

	bool changed = false;

	BeginPropertyRow();

	ImGui::Text(text.c_str());

	ImGui::TableNextColumn();

	const auto width = ImGui::CalcItemWidth() / 3;

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

	changed |= DrawItem(width, [&]()
	{
		return ImGui::DragFloat(id.c_str(), &value.x, 0.1f);
	});

	if (ImGui::IsItemHovered())
	{
		if (Volt::Input::IsMouseButtonDown(VT_MOUSE_BUTTON_LEFT))
		{
			changed = true;
		}
	}

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

	changed |= DrawItem(width, [&]()
	{
		return ImGui::DragFloat(id.c_str(), &value.y, 0.1f);
	});

	if (ImGui::IsItemHovered())
	{
		if (Volt::Input::IsMouseButtonDown(VT_MOUSE_BUTTON_LEFT))
		{
			changed = true;
		}
	}

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
		}
	}

	ImGui::SameLine();
	id = "##" + std::to_string(s_stackId++);

	changed |= DrawItem(width, [&]()
	{
		return ImGui::DragFloat(id.c_str(), &value.z, 0.1f);
	});

	if (ImGui::IsItemHovered())
	{
		if (Volt::Input::IsMouseButtonDown(VT_MOUSE_BUTTON_LEFT))
		{
			changed = true;
		}
	}

	ImGui::PopStyleVar();

	EndPropertyRow();

	return changed;
}

bool UI::PropertyAxisColor(const std::string& text, glm::vec2& value, float resetValue)
{
	ScopedStyleFloat2 cellPad(ImGuiStyleVar_CellPadding, { 4.f, 0.f });

	bool changed = false;

	BeginPropertyRow();

	ImGui::Text(text.c_str());

	ImGui::TableNextColumn();
	const float width = ImGui::CalcItemWidth() / 2;

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

	changed |= DrawItem(width, [&]()
	{
		return ImGui::DragFloat(id.c_str(), &value.x, 0.1f);
	});

	if (ImGui::IsItemHovered())
	{
		if (Volt::Input::IsMouseButtonDown(VT_MOUSE_BUTTON_LEFT))
		{
			changed = true;
		}
	}

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

	changed |= DrawItem(width, [&]()
	{
		return ImGui::DragFloat(id.c_str(), &value.y, 0.1f);
	});

	if (ImGui::IsItemHovered())
	{
		if (Volt::Input::IsMouseButtonDown(VT_MOUSE_BUTTON_LEFT))
		{
			changed = true;
		}
	}

	ImGui::PopStyleVar();

	EndPropertyRow();

	return changed;
}

bool UI::Property(const std::string& text, bool& value, const std::string& toolTip)
{
	bool changed = false;

	BeginPropertyRow();

	ImGui::TextUnformatted(text.c_str());
	SimpleToolTip(toolTip);

	ImGui::TableNextColumn();
	std::string id = "##" + std::to_string(s_stackId++);

	changed = DrawItem([&]()
	{
		return ImGui::Checkbox(id.c_str(), &value);
	});

	EndPropertyRow();

	return changed;
}

bool UI::Property(const std::string& text, int32_t& value, const std::string& toolTip)
{
	bool changed = false;

	BeginPropertyRow();

	ImGui::TextUnformatted(text.c_str());
	SimpleToolTip(toolTip);

	ImGui::TableNextColumn();
	std::string id = "##" + std::to_string(s_stackId++);

	changed = DrawItem([&id, &value]()
	{
		return ImGui::DragScalar(id.c_str(), ImGuiDataType_S32, (void*)&value, 1.f);
	});

	EndPropertyRow();

	return changed;
}

bool UI::Property(const std::string& text, uint32_t& value, const std::string& toolTip)
{
	bool changed = false;

	BeginPropertyRow();

	ImGui::TextUnformatted(text.c_str());
	SimpleToolTip(toolTip);

	ImGui::TableNextColumn();
	std::string id = "##" + std::to_string(s_stackId++);

	changed = DrawItem([&]()
	{
		return ImGui::DragScalar(id.c_str(), ImGuiDataType_U32, (void*)&value, 1.f);
	});

	EndPropertyRow();

	return changed;
}

bool UI::Property(const std::string& text, int16_t& value, const std::string& toolTip)
{
	bool changed = false;

	BeginPropertyRow();

	ImGui::TextUnformatted(text.c_str());
	SimpleToolTip(toolTip);

	ImGui::TableNextColumn();
	std::string id = "##" + std::to_string(s_stackId++);

	changed = DrawItem([&]()
	{
		return ImGui::DragScalar(id.c_str(), ImGuiDataType_S16, (void*)&value, 1.f);
	});

	EndPropertyRow();

	return changed;
}

bool UI::Property(const std::string& text, uint16_t& value, const std::string& toolTip)
{
	bool changed = false;

	BeginPropertyRow();

	ImGui::TextUnformatted(text.c_str());
	SimpleToolTip(toolTip);

	ImGui::TableNextColumn();
	std::string id = "##" + std::to_string(s_stackId++);

	changed = DrawItem([&]()
	{
		return ImGui::DragScalar(id.c_str(), ImGuiDataType_U16, (void*)&value, 1.f);
	});

	EndPropertyRow();

	return changed;
}

bool UI::Property(const std::string& text, int8_t& value, const std::string& toolTip)
{
	bool changed = false;

	BeginPropertyRow();

	ImGui::TextUnformatted(text.c_str());
	SimpleToolTip(toolTip);

	ImGui::TableNextColumn();
	std::string id = "##" + std::to_string(s_stackId++);

	changed = DrawItem([&]()
	{
		return ImGui::DragScalar(id.c_str(), ImGuiDataType_S8, (void*)&value, 1.f);
	});

	EndPropertyRow();
	return changed;
}

bool UI::Property(const std::string& text, uint8_t& value, const std::string& toolTip)
{
	bool changed = false;

	BeginPropertyRow();

	ImGui::TextUnformatted(text.c_str());
	SimpleToolTip(toolTip);

	ImGui::TableNextColumn();
	std::string id = "##" + std::to_string(s_stackId++);

	changed = DrawItem([&]()
	{
		return ImGui::DragScalar(id.c_str(), ImGuiDataType_U8, (void*)&value, 1.f);
	});

	EndPropertyRow();

	return changed;
}

bool UI::Property(const std::string& text, double& value, const std::string& toolTip)
{
	bool changed = false;

	BeginPropertyRow();

	ImGui::TextUnformatted(text.c_str());
	SimpleToolTip(toolTip);

	ImGui::TableNextColumn();
	std::string id = "##" + std::to_string(s_stackId++);

	changed = DrawItem([&]()
	{
		return ImGui::DragScalar(id.c_str(), ImGuiDataType_Double, (void*)&value, 1.f);
	});

	EndPropertyRow();
	return changed;
}

bool UI::Property(const std::string& text, float& value, float min, float max, const std::string& toolTip)
{
	bool changed = false;

	BeginPropertyRow();

	ImGui::TextUnformatted(text.c_str());
	SimpleToolTip(toolTip);

	ImGui::TableNextColumn();
	std::string id = "##" + std::to_string(s_stackId++);

	changed = DrawItem([&]()
	{
		bool c = false;

		if (min != 0.f && max != 0.f)
		{
			c = ImGui::SliderFloat(id.c_str(), &value, min, max);
		}
		else
		{
			c = ImGui::DragFloat(id.c_str(), &value, 0.1f);
		}

		return c;
	});

	EndPropertyRow();

	return changed;
}

bool UI::Property(const std::string& text, glm::vec2& value, float min, float max, const std::string& toolTip)
{
	bool changed = false;

	BeginPropertyRow();

	ImGui::TextUnformatted(text.c_str());
	SimpleToolTip(toolTip);

	ImGui::TableNextColumn();
	std::string id = "##" + std::to_string(s_stackId++);

	changed = DragScalarN(id.c_str(), ImGuiDataType_Float, glm::value_ptr(value), 2, 0.1f, &min, &max);

	EndPropertyRow();

	return changed;
}

bool UI::Property(const std::string& text, glm::vec3& value, float min, float max, const std::string& toolTip)
{
	bool changed = false;

	BeginPropertyRow();

	ImGui::TextUnformatted(text.c_str());
	SimpleToolTip(toolTip);

	ImGui::TableNextColumn();
	std::string id = "##" + std::to_string(s_stackId++);

	changed = DragScalarN(id.c_str(), ImGuiDataType_Float, glm::value_ptr(value), 3, 0.1f, &min, &max);

	EndPropertyRow();

	return changed;
}

bool UI::Property(const std::string& text, glm::vec4& value, float min, float max, const std::string& toolTip)
{
	bool changed = false;

	BeginPropertyRow();

	ImGui::TextUnformatted(text.c_str());
	SimpleToolTip(toolTip);

	ImGui::TableNextColumn();
	std::string id = "##" + std::to_string(s_stackId++);

	changed = DragScalarN(id.c_str(), ImGuiDataType_Float, glm::value_ptr(value), 4, 0.1f, &min, &max);
	EndPropertyRow();

	return changed;
}

bool UI::Property(const std::string& text, glm::uvec2& value, uint32_t min, uint32_t max, const std::string& toolTip)
{
	bool changed = false;

	BeginPropertyRow();

	ImGui::TextUnformatted(text.c_str());
	SimpleToolTip(toolTip);

	ImGui::TableNextColumn();
	std::string id = "##" + std::to_string(s_stackId++);

	changed = DragScalarN(id.c_str(), ImGuiDataType_U32, glm::value_ptr(value), 2, 1.f, &min, &max);

	EndPropertyRow();

	return changed;
}

bool UI::Property(const std::string& text, glm::uvec3& value, uint32_t min, uint32_t max, const std::string& toolTip)
{
	bool changed = false;

	BeginPropertyRow();

	ImGui::TextUnformatted(text.c_str());
	SimpleToolTip(toolTip);

	ImGui::TableNextColumn();
	std::string id = "##" + std::to_string(s_stackId++);

	changed = DragScalarN(id.c_str(), ImGuiDataType_U32, glm::value_ptr(value), 3, 1.f, &min, &max);

	EndPropertyRow();

	return changed;
}

bool UI::Property(const std::string& text, glm::uvec4& value, uint32_t min, uint32_t max, const std::string& toolTip)
{
	bool changed = false;

	BeginPropertyRow();

	ImGui::TextUnformatted(text.c_str());
	SimpleToolTip(toolTip);

	ImGui::TableNextColumn();
	std::string id = "##" + std::to_string(s_stackId++);

	changed = DragScalarN(id.c_str(), ImGuiDataType_U32, glm::value_ptr(value), 4, 1.f, &min, &max);
	EndPropertyRow();

	return changed;
}

bool UI::Property(const std::string& text, glm::ivec2& value, uint32_t min, uint32_t max, const std::string& toolTip)
{
	bool changed = false;

	BeginPropertyRow();

	ImGui::TextUnformatted(text.c_str());
	SimpleToolTip(toolTip);

	ImGui::TableNextColumn();
	std::string id = "##" + std::to_string(s_stackId++);

	changed = DragScalarN(id.c_str(), ImGuiDataType_S32, glm::value_ptr(value), 2, 1.f, &min, &max);

	EndPropertyRow();

	return changed;
}

bool UI::Property(const std::string& text, glm::ivec3& value, uint32_t min, uint32_t max, const std::string& toolTip)
{
	bool changed = false;

	BeginPropertyRow();

	ImGui::TextUnformatted(text.c_str());
	SimpleToolTip(toolTip);

	ImGui::TableNextColumn();
	std::string id = "##" + std::to_string(s_stackId++);

	changed = DragScalarN(id.c_str(), ImGuiDataType_S32, glm::value_ptr(value), 3, 1.f, &min, &max);
	EndPropertyRow();

	return changed;
}

bool UI::Property(const std::string& text, glm::ivec4& value, uint32_t min, uint32_t max, const std::string& toolTip)
{
	bool changed = false;

	BeginPropertyRow();

	ImGui::TextUnformatted(text.c_str());
	SimpleToolTip(toolTip);

	ImGui::TableNextColumn();
	std::string id = "##" + std::to_string(s_stackId++);

	changed = DragScalarN(id.c_str(), ImGuiDataType_S32, glm::value_ptr(value), 4, 1.f, &min, &max);
	EndPropertyRow();

	return changed;
}

bool UI::Property(const std::string& text, glm::quat& value, const std::string& toolTip)
{
	bool changed = false;

	BeginPropertyRow();

	ImGui::TextUnformatted(text.c_str());
	SimpleToolTip(toolTip);

	ImGui::TableNextColumn();
	std::string id = "##" + std::to_string(s_stackId++);

	constexpr float min = -1.f;
	constexpr float max = 1.f;

	changed = DragScalarN(id.c_str(), ImGuiDataType_Float, glm::value_ptr(value), 4, 1.f, &min, &max);
	EndPropertyRow();

	return changed;
}

bool UI::PropertyDragFloat(const std::string& text, float& value, float increment, float min, float max, const std::string& toolTip)
{
	bool changed = false;

	BeginPropertyRow();

	ImGui::TextUnformatted(text.c_str());
	SimpleToolTip(toolTip);

	ImGui::TableNextColumn();
	std::string id = "##" + std::to_string(s_stackId++);

	changed = DrawItem([&]()
	{
		return ImGui::DragFloat(id.c_str(), &value, increment, min, max);
	});

	EndPropertyRow();

	return changed;
}

bool UI::PropertyTextBox(const std::string& text, const std::string& value, bool readOnly, const std::string& toolTip)
{
	bool changed = false;

	BeginPropertyRow();

	ImGui::TextUnformatted(text.c_str());
	SimpleToolTip(toolTip);
	ImGui::TableNextColumn();

	changed = DrawItem([&]()
	{
		return InputText("", const_cast<std::string&>(value), readOnly ? ImGuiInputTextFlags_ReadOnly : ImGuiInputTextFlags_None);
	});

	EndPropertyRow();

	return changed;
}

bool UI::PropertyEntity(const std::string& text, Weak<Volt::Scene> scene, Volt::EntityID& value, const std::string& toolTip)
{
	bool changed = false;

	BeginPropertyRow();

	ImGui::TextUnformatted(text.c_str());
	SimpleToolTip(toolTip);

	ImGui::TableNextColumn();
	std::string id = "##" + std::to_string(s_stackId++);

	Volt::Entity entity = scene->GetEntityFromUUID(value);

	std::string entityName;
	if (entity)
	{
		entityName = entity.GetComponent<Volt::TagComponent>().tag;
	}
	else
	{
		entityName = "Null";
	}

	changed = DrawItem([&]()
	{
		return ImGui::InputTextString(id.c_str(), &entityName, ImGuiInputTextFlags_ReadOnly);
	});

	if (auto ptr = UI::DragDropTarget("scene_entity_hierarchy"))
	{
		Volt::EntityID entityId = *(Volt::EntityID*)ptr;
		value = entityId;
		changed = true;
	}

	EndPropertyRow();

	return changed;
}

bool UI::PropertyEntity(Weak<Volt::Scene> scene, Volt::EntityID& value, const float width, const std::string& toolTip)
{
	bool changed = false;

	SimpleToolTip(toolTip);
	std::string id = "##" + std::to_string(s_stackId++);

	Volt::Entity entity = scene->GetEntityFromUUID(value);

	std::string entityName;
	if (entity)
	{
		entityName = entity.GetComponent<Volt::TagComponent>().tag;
	}
	else
	{
		entityName = "Null";
	}

	changed = DrawItem(width, [&]()
	{
		return ImGui::InputTextString(id.c_str(), &entityName, ImGuiInputTextFlags_ReadOnly);
	});

	if (auto ptr = UI::DragDropTarget("scene_entity_hierarchy"))
	{
		Volt::EntityID entityId = *(Volt::EntityID*)ptr;
		value = entityId;
		changed = true;
	}

	EndPropertyRow();

	return changed;
}

bool UI::PropertyEntityCustomMonoType(const std::string& text, Weak<Volt::Scene> scene, Volt::EntityID& value, const Volt::MonoTypeInfo& monoTypeInfo, const std::string& toolTip)
{
	bool changed = false;

	BeginPropertyRow();

	ImGui::TextUnformatted(text.c_str());
	SimpleToolTip(toolTip);

	ImGui::TableNextColumn();
	std::string id = "##" + std::to_string(s_stackId++);

	Volt::Entity entity = scene->GetEntityFromUUID(value);

	std::string entityName;
	if (entity)
	{
		entityName = entity.GetComponent<Volt::TagComponent>().tag;
	}
	else
	{
		entityName = "Null";
	}

	changed = DrawItem([&]()
		{
			return ImGui::InputTextString(id.c_str(), &entityName, ImGuiInputTextFlags_ReadOnly);
		});

	if (auto ptr = UI::DragDropTarget("scene_entity_hierarchy"))
	{
		Volt::EntityID entityId = *(Volt::EntityID*)ptr;
		auto droppedEntity = scene->GetEntityFromUUID(entityId);

		// Check that the dropped entity has the required script
		if (droppedEntity.HasComponent<Volt::MonoScriptComponent>())
		{
			const auto& monoComponent = droppedEntity.GetComponent<Volt::MonoScriptComponent>();

			for (const auto& scriptName : monoComponent.scriptNames)
			{
				if (scriptName == monoTypeInfo.typeName)
				{
					value = entityId;
					changed = true;
					break;
				}
			}
		}

	}

	EndPropertyRow();

	return changed;
}

bool UI::Property(const std::string& text, const std::string& value, bool readOnly, const std::string& toolTip)
{
	bool changed = false;

	BeginPropertyRow();

	ImGui::TextUnformatted(text.c_str());
	SimpleToolTip(toolTip);
	ImGui::TableNextColumn();

	changed = DrawItem([&]()
	{
		return InputText("", const_cast<std::string&>(value), readOnly ? ImGuiInputTextFlags_ReadOnly : ImGuiInputTextFlags_None);
	});

	EndPropertyRow();

	return changed;
}

bool UI::Property(const std::string& text, std::string& value, bool readOnly, const std::string& toolTip)
{
	bool changed = false;

	BeginPropertyRow();

	ImGui::TextUnformatted(text.c_str());
	SimpleToolTip(toolTip);
	ImGui::TableNextColumn();

	changed = DrawItem([&]()
	{
		return InputText("", value, readOnly ? ImGuiInputTextFlags_ReadOnly : ImGuiInputTextFlags_None);
	});

	EndPropertyRow();
	return changed;
}

bool UI::PropertyColor(const std::string& text, glm::vec4& value, const std::string& toolTip)
{
	BeginPropertyRow();

	ImGui::TextUnformatted(text.c_str());
	SimpleToolTip(toolTip);
	ImGui::TableNextColumn();
	std::string id = "##" + std::to_string(s_stackId++);

	bool changed = DrawItem([&]()
	{
		return ImGui::ColorEdit4(id.c_str(), glm::value_ptr(value));
	});

	EndPropertyRow();
	return changed;
}

bool UI::PropertyColor(const std::string& text, glm::vec3& value, const std::string& toolTip)
{
	BeginPropertyRow();

	ImGui::TextUnformatted(text.c_str());
	SimpleToolTip(toolTip);
	ImGui::TableNextColumn();
	std::string id = "##" + std::to_string(s_stackId++);

	bool changed = DrawItem([&]()
	{
		return ImGui::ColorEdit3(id.c_str(), glm::value_ptr(value));
	});

	EndPropertyRow();
	return changed;
}

bool UI::Property(const std::string& text, std::filesystem::path& path, const std::string& toolTip)
{
	bool changed = false;

	BeginPropertyRow();

	ImGui::TextUnformatted(text.c_str());
	SimpleToolTip(toolTip);

	ImGui::TableNextColumn();
	std::string sPath = path.string();

	changed = DrawItem(ImGui::GetColumnWidth() - ImGui::CalcTextSize("Open...").x - 20.f, [&]()
	{
		if (InputText("", sPath))
		{
			path = std::filesystem::path(sPath);
			return true;
		}

		return false;
	});

	ImGui::SameLine();

	std::string buttonId = "Open...##" + std::to_string(s_stackId++);
	if (ImGui::Button(buttonId.c_str(), { ImGui::GetContentRegionAvail().x, 25.f }))
	{
		auto newPath = FileSystem::OpenFileDialogue({ { "All (*.*)" }, { "*" } });
		if (!newPath.empty())
		{
			path = newPath;
			changed = true;
		}
	}

	EndPropertyRow();

	return changed;
}
bool UI::PropertyDirectory(const std::string& text, std::filesystem::path& path, const std::string& toolTip)
{
	bool changed = false;

	BeginPropertyRow();

	ImGui::TextUnformatted(text.c_str());
	SimpleToolTip(toolTip);
	ImGui::TableNextColumn();
	std::string sPath = path.string();

	changed = DrawItem(ImGui::GetColumnWidth() - ImGui::CalcTextSize("Open...").x - 20.f, [&]()
	{
		if (InputText("", sPath))
		{
			path = std::filesystem::path(sPath);
			return true;
		}

		return false;
	});

	ImGui::SameLine();

	std::string buttonId = "Open...##" + std::to_string(s_stackId++);
	if (ImGui::Button(buttonId.c_str(), { ImGui::GetContentRegionAvail().x, 25.f }))
	{
		auto newPath = FileSystem::PickFolderDialogue();
		if (!newPath.empty())
		{
			path = newPath;
			changed = true;
		}
	}

	EndPropertyRow();

	return changed;
}
bool UI::PropertyMultiline(const std::string& text, std::string& value, bool readOnly, const std::string& toolTip)
{
	bool changed = false;

	BeginPropertyRow();

	ImGui::TextUnformatted(text.c_str());
	SimpleToolTip(toolTip);
	ImGui::TableNextColumn();

	changed = DrawItem([&]()
	{
		return InputTextMultiline("", value, readOnly ? ImGuiInputTextFlags_ReadOnly : ImGuiInputTextFlags_None);
	});

	EndPropertyRow();

	return changed;
}

bool UI::PropertyPassword(const std::string& text, std::string& value, bool readOnly, const std::string& toolTip)
{
	bool changed = false;

	BeginPropertyRow();

	ImGui::TextUnformatted(text.c_str());
	SimpleToolTip(toolTip);
	ImGui::TableNextColumn();

	changed = DrawItem([&]()
	{
		return InputText("", value, readOnly ? ImGuiInputTextFlags_ReadOnly : ImGuiInputTextFlags_Password);
	});

	EndPropertyRow();
	return changed;
}
