#pragma once

#include "Volt/Core/Base.h"
#include "Volt/Utility/FileSystem.h"
#include "Volt/Utility/ImGuiExtension.h"

#include "Volt/Asset/AssetManager.h"
#include "Volt/Core/Application.h"

#include "Volt/Input/Input.h"
#include "Volt/Input/MouseButtonCodes.h"
#include "Volt/Scene/Scene.h"
#include "Volt/Scene/Entity.h"

#include "Volt/Components/CoreComponents.h"
#include "Volt/ImGui/ImGuiImplementation.h"

#include <glm/glm.hpp>

#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_stdlib.h>
#include <imgui_notify.h>

namespace Volt
{
	class Texture2D;
	class Image2D;
}

enum class NotificationType
{
	Info,
	Warning,
	Error,
	Success
};

enum class FontType
{
	Regular_12,
	Regular_16,
	Regular_17,
	Regular_20,
	Bold_12,
	Bold_16,
	Bold_17,
	Bold_20,
	Bold_90
};

class UI
{
public:
	struct Button
	{
		glm::vec4 normal = { 1.f, 1.f, 1.f, 1.f };
		glm::vec4 hovered = { 1.f, 1.f, 1.f, 1.f };
		glm::vec4 active = { 1.f, 1.f, 1.f, 1.f };
	};

	class ScopedButtonColor
	{
	public:
		ScopedButtonColor(const Button& newColors)
		{
			auto& colors = ImGui::GetStyle().Colors;
			myOldNormalColor = colors[ImGuiCol_Button];
			myOldHoveredColor = colors[ImGuiCol_ButtonHovered];
			myOldActiveColor = colors[ImGuiCol_ButtonActive];

			colors[ImGuiCol_Button] = ImVec4{ newColors.normal.x, newColors.normal.y, newColors.normal.z, newColors.normal.w };
			colors[ImGuiCol_ButtonHovered] = ImVec4{ newColors.hovered.x, newColors.hovered.y, newColors.hovered.z, newColors.hovered.w };
			colors[ImGuiCol_ButtonActive] = ImVec4{ newColors.active.x, newColors.active.y, newColors.active.z, newColors.active.w };
		}

		~ScopedButtonColor()
		{
			auto& colors = ImGui::GetStyle().Colors;
			colors[ImGuiCol_Button] = myOldNormalColor;
			colors[ImGuiCol_ButtonHovered] = myOldHoveredColor;
			colors[ImGuiCol_ButtonActive] = myOldActiveColor;
		}

	private:
		ImVec4 myOldNormalColor;
		ImVec4 myOldHoveredColor;
		ImVec4 myOldActiveColor;
	};

	class ScopedColor
	{
	public:
		ScopedColor(ImGuiCol_ color, const glm::vec4& newColor)
			: myColor(color)
		{
			auto& colors = ImGui::GetStyle().Colors;
			myOldColor = colors[color];
			colors[color] = ImVec4{ newColor.x, newColor.y, newColor.z, newColor.w };
		}

		~ScopedColor()
		{
			auto& colors = ImGui::GetStyle().Colors;
			colors[myColor] = myOldColor;
		}

	private:
		ImVec4 myOldColor;
		ImGuiCol_ myColor;
	};

	class ScopedColorPrediacate
	{
	public:
		ScopedColorPrediacate(bool predicate, ImGuiCol_ color, const glm::vec4& newColor)
			: myColor(color), myPredicate(predicate)
		{
			if (!myPredicate)
			{
				return;
			}

			auto& colors = ImGui::GetStyle().Colors;
			myOldColor = colors[color];
			colors[color] = ImVec4{ newColor.x, newColor.y, newColor.z, newColor.w };
		}

		~ScopedColorPrediacate()
		{
			if (!myPredicate)
			{
				return;
			}

			auto& colors = ImGui::GetStyle().Colors;
			colors[myColor] = myOldColor;
		}

	private:
		ImVec4 myOldColor;
		ImGuiCol_ myColor;

		bool myPredicate = false;
	};

	class ScopedStyleFloat
	{
	public:
		ScopedStyleFloat(ImGuiStyleVar_ var, float value)
		{
			ImGui::PushStyleVar(var, value);
		}

		~ScopedStyleFloat()
		{
			ImGui::PopStyleVar();
		}
	};

	class ScopedStyleFloat2
	{
	public:
		ScopedStyleFloat2(ImGuiStyleVar_ var, const glm::vec2& value)
		{
			ImGui::PushStyleVar(var, { value.x, value.y });
		}

		~ScopedStyleFloat2()
		{
			ImGui::PopStyleVar();
		}
	};

	class ScopedFont
	{
	public:
		inline ScopedFont(FontType font)
		{
			ImGui::PushFont(s_fonts.at(font));
		}

		inline ~ScopedFont()
		{
			ImGui::PopFont();
		}

	private:
	};

	static ImTextureID GetTextureID(Ref<Volt::Texture2D> texture);
	static ImTextureID GetTextureID(Ref<Volt::Image2D> texture);
	static ImTextureID GetTextureID(Volt::Texture2D* texture);

	static void Header(const std::string& text);

	static ImGuiToastType ToastTypeFromNotificationType(NotificationType type);

	static void ShiftCursor(float x, float y);

	static bool BeginPopup(const std::string& name, ImGuiWindowFlags flags = 0);
	static bool BeginPopupItem(const std::string& id = "", ImGuiPopupFlags flags = 0);
	static bool BeginPopupWindow(const std::string& id = "");
	static void EndPopup();

	static bool InputText(const std::string& name, std::string& text, ImGuiInputTextFlags_ flags = ImGuiInputTextFlags_None);
	static bool InputTextWithHint(const std::string& name, std::string& text, const std::string& hint, ImGuiInputTextFlags_ flags = ImGuiInputTextFlags_None);
	static bool InputTextMultiline(const std::string& name, std::string& text, ImGuiInputTextFlags_ flags = ImGuiInputTextFlags_None);

	static bool ImageButton(const std::string& id, ImTextureID textureId, const ImVec2& size, const ImVec4& bg_col, const ImVec4& tint_col);
	static bool ImageButton(const std::string& id, ImTextureID textureId, const ImVec2& size, const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1), int frame_padding = -1, const ImVec4& bg_col = ImVec4(0, 0, 0, 0), const ImVec4& tint_col = ImVec4(1, 1, 1, 1));
	static bool ImageButtonState(const std::string& id, bool state, ImTextureID textureId, const ImVec2& size, const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1));

	static bool TreeNodeImage(Ref<Volt::Texture2D> texture, const std::string& text, ImGuiTreeNodeFlags flags, bool setOpen = false);
	static bool TreeNodeFramed(const std::string& text, bool alwaysOpen = false, float rounding = 0.f);
	static bool TreeNodeWidth(const std::string& text, float width, float rounding = 0.f, ImGuiTreeNodeFlags flags = 0);
	static void TreeNodePop();

	static bool CollapsingHeader(std::string_view label, ImGuiTreeNodeFlags flags = 0);

	static void SameLine(float offsetX = 0.f, float spacing = -1.f);

	static bool ImageSelectable(Ref<Volt::Texture2D> texture, const std::string& text, bool& selected);
	static bool ImageSelectable(Ref<Volt::Texture2D> texture, const std::string& text);
	static bool ImageSelectable(Ref<Volt::Texture2D> texture, const std::string& text, bool selected);

	static void PushID();
	static void PopID();
	static int32_t GetID();

	static bool IsInputEnabled();
	static void SetInputEnabled(bool enable);

	static void SimpleToolTip(const std::string& toolTip);

	static bool BeginMenuBar(ImRect barRect);
	static void EndMenuBar();

	static bool BeginListView(const std::string& strId);
	static void EndListView();

	static bool BeginProperties(const std::string& name = "", const ImVec2 size = { 0, 0 });
	static void EndProperties();

	static bool ComboProperty(const std::string& text, int& currentItem, const std::vector<std::string>& strItems, float width = 0.f);
	static bool ComboProperty(const std::string& text, int& currentItem, const std::vector<const char*>& items, float width = 0.f);

	static bool Combo(const std::string& text, int& currentItem, const std::vector<const char*>& items, float width = 100.f);
	static bool Combo(const std::string& text, int& currentItem, const char** items, uint32_t count);
	static bool Combo(const std::string& text, int& currentItem, const std::vector<std::string>& strItems, float width = 100.f);

	static void* DragDropTarget(const std::string& type);
	static void* DragDropTarget(std::initializer_list<std::string> types, ImGuiDragDropFlags flags = 0);

	static void Notify(NotificationType type, const std::string& title, const std::string& content, int32_t duration = 5000);

	static void OpenModal(const std::string& name, ImGuiPopupFlags flags = 0);
	static void OpenPopup(const std::string& name, ImGuiPopupFlags flags = 0);

	static bool BeginModal(const std::string& name, ImGuiWindowFlags flags = 0);
	static void EndModal();

	static void SmallSeparatorHeader(const std::string& text, float padding);

	static void PropertyInfoString(const std::string& key, const std::string& info);
	static bool PropertyAxisColor(const std::string& text, glm::vec3& value, float resetValue = 0.f);
	static bool PropertyAxisColor(const std::string& text, glm::vec2& value, float resetValue = 0.f);
	
	static bool PropertyEntity(const std::string& text, Weak<Volt::Scene> scene, Volt::EntityID& value, const std::string& toolTip = "");
	static bool PropertyEntity(Weak<Volt::Scene> scene, Volt::EntityID& value, const float width, const std::string& toolTip = "");

	static bool PropertyDragFloat(const std::string& text, float& value, float increment, float min = 0.f, float max = 0.f, const std::string& toolTip = "");
	static bool PropertyTextBox(const std::string& text, const std::string& value, bool readOnly = false, const std::string& toolTip = "");
	static bool PropertyPassword(const std::string& text, std::string& value, bool readOnly = false, const std::string& toolTip = "");
	static bool PropertyMultiline(const std::string& text, std::string& value, bool readOnly = false, const std::string& toolTip = "");
	static bool PropertyDirectory(const std::string& text, std::filesystem::path& path, const std::string& toolTip = "");

	static bool PropertyColor(const std::string& text, glm::vec4& value, const std::string& toolTip = "");
	static bool PropertyColor(const std::string& text, glm::vec3& value, const std::string& toolTip = "");

	static bool Property(const std::string& text, bool& value, const std::string& toolTip = "");
	
	static bool Property(const std::string& text, int32_t& value, const std::string& toolTip = "");
	static bool Property(const std::string& text, uint32_t& value, const std::string& toolTip = "");
	static bool Property(const std::string& text, int16_t& value, const std::string& toolTip = "");
	static bool Property(const std::string& text, uint16_t& value, const std::string& toolTip = "");
	static bool Property(const std::string& text, int8_t& value, const std::string& toolTip = "");
	static bool Property(const std::string& text, uint8_t& value, const std::string& toolTip = "");

	static bool Property(const std::string& text, double& value, const std::string& toolTip = "");
	static bool Property(const std::string& text, float& value, float min = 0.f, float max = 0.f, const std::string& toolTip = "");

	static bool Property(const std::string& text, glm::vec2& value, float min = 0.f, float max = 0.f, const std::string& toolTip = "");
	static bool Property(const std::string& text, glm::vec3& value, float min = 0.f, float max = 0.f, const std::string& toolTip = "");
	static bool Property(const std::string& text, glm::vec4& value, float min = 0.f, float max = 0.f, const std::string& toolTip = "");
	
	static bool Property(const std::string& text, glm::uvec2& value, uint32_t min = 0, uint32_t max = 0, const std::string& toolTip = "");
	static bool Property(const std::string& text, glm::uvec3& value, uint32_t min = 0, uint32_t max = 0, const std::string& toolTip = "");
	static bool Property(const std::string& text, glm::uvec4& value, uint32_t min = 0, uint32_t max = 0, const std::string& toolTip = "");

	static bool Property(const std::string& text, glm::ivec2& value, uint32_t min = 0, uint32_t max = 0, const std::string& toolTip = "");
	static bool Property(const std::string& text, glm::ivec3& value, uint32_t min = 0, uint32_t max = 0, const std::string& toolTip = "");
	static bool Property(const std::string& text, glm::ivec4& value, uint32_t min = 0, uint32_t max = 0, const std::string& toolTip = "");

	static bool Property(const std::string& text, const std::string& value, bool readOnly = false, const std::string& toolTip = "");
	static bool Property(const std::string& text, std::string& value, bool readOnly = false, const std::string& toolTip = "");
	static bool Property(const std::string& text, std::filesystem::path& path, const std::string& toolTip = "");

	template<typename T>
	static bool Property(const std::string& text, Ref<T>& asset, const std::string& toolTip = "")
	{
		bool changed = false;

		ImGui::TableNextColumn();
		ImGui::TextUnformatted(text.c_str());
		SimpleToolTip(toolTip);

		ImGui::TableNextColumn();

		ImGui::PushItemWidth(ImGui::GetColumnWidth() - 20.f);

		std::string assetFileName = "Null";

		if (asset)
		{
			assetFileName = asset->path.filename().string();
		}

		std::string textId = "##" + std::to_string(s_stackId++);
		ImGui::InputTextString(textId.c_str(), &assetFileName, ImGuiInputTextFlags_ReadOnly);
		ImGui::PopItemWidth();

		if (auto ptr = UI::DragDropTarget("ASSET_BROWSER_ITEM"))
		{
			Volt::AssetHandle newHandle = *(Volt::AssetHandle*)ptr;
			Ref<T> newAsset = Volt::AssetManager::GetAsset<T>(newHandle);
			if (newAsset)
			{
				asset = newAsset;
			}
			else
			{
				asset = nullptr;
			}

			changed = true;
		}

		ImGui::SameLine();

		std::string buttonId = "X##" + std::to_string(s_stackId++);
		if (ImGui::Button(buttonId.c_str()))
		{
			asset = nullptr;
			changed = true;
		}

		return changed;
	}

	inline static void SetFont(FontType type, ImFont* font)
	{
		s_fonts[type] = font;
	}

	static void PushFont(FontType font);
	static void PopFont();

	static int32_t LevenshteinDistance(const std::string& str1, const std::string& str2);
	static const std::vector<std::string> GetEntriesMatchingQuery(const std::string& query, const std::vector<std::string>& entries);

	static void RenderMatchingTextBackground(const std::string& query, const std::string& text, const glm::vec4& color, const glm::uvec2& offset = 0u);
	static void RenderHighlightedBackground(const glm::vec4& color, float height);

	static void BeginPropertyRow();
	static void EndPropertyRow();

	static bool DrawItem(std::function<bool()> itemFunc);
	static bool DrawItem(const float itemWidth, std::function<bool()> itemFunc);

private:
	static bool IsPropertyRowHovered();
	static bool IsPropertyColumnHovered(const uint32_t column);
	static void SetPropertyBackgroundColor();
	static void SetRowColor(const glm::vec4& color);

	static bool IsItemHovered(const float itemWidth);

	static bool DragScalarN(const std::string& id, ImGuiDataType dataType, void* data, int32_t components, float speed, const void* min, const void* max);

	inline static uint32_t s_contextId = 0;
	inline static uint32_t s_stackId = 0;
	inline static std::unordered_map<FontType, ImFont*> s_fonts;

};
