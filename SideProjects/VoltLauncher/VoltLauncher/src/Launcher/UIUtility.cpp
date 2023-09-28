#include "UIUtility.h"

#include "imgui_internal.h"

#include "FileSystem.h"

namespace UI
{
	inline static uint32_t s_contextId = 0;
	inline static uint32_t s_stackId = 0;

	inline static constexpr float PROPERTY_ROW_HEIGHT = 17.f;
	inline static constexpr float PROPERTY_ROW_PADDING = 4.f;

	void SimpleToolTip(const std::string& toolTip)
	{
		if (!toolTip.empty())
		{
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("%s", toolTip.c_str());
			}
		}
	}

	bool InputText(const std::string& name, std::string& text, ImGuiInputTextFlags_ flags)
	{
		if (!name.empty())
		{
			ImGui::TextUnformatted(name.c_str());
			ImGui::SameLine();
		}

		std::string id = "##" + std::to_string(s_stackId++);
		return InputTextString(id.c_str(), &text, flags);
	}

	struct InputTextCallback_UserData
	{
		std::string* Str;
		ImGuiInputTextCallback  ChainCallback;
		void* ChainCallbackUserData;
	};

	static int InputTextCallback(ImGuiInputTextCallbackData* data)
	{
		InputTextCallback_UserData* user_data = (InputTextCallback_UserData*)data->UserData;
		if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
		{
			// Resize string callback
			// If for some reason we refuse the new length (BufTextLen) and/or capacity (BufSize) we need to set them back to what we want.
			std::string* str = user_data->Str;
			IM_ASSERT(data->Buf == str->c_str());
			str->resize(data->BufTextLen);
			data->Buf = (char*)str->c_str();
		}
		else if (user_data->ChainCallback)
		{
			// Forward to user callback, if any
			data->UserData = user_data->ChainCallbackUserData;
			return user_data->ChainCallback(data);
		}
		return 0;
	}

	bool InputTextString(const char* label, std::string* str, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data)
	{
		IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
		flags |= ImGuiInputTextFlags_CallbackResize;

		InputTextCallback_UserData cb_user_data;
		cb_user_data.Str = str;
		cb_user_data.ChainCallback = callback;
		cb_user_data.ChainCallbackUserData = user_data;
		return ImGui::InputText(label, (char*)str->c_str(), str->capacity() + 1, flags, InputTextCallback, &cb_user_data);
	}

	void PushID()
	{
		int id = s_contextId++;
		ImGui::PushID(id);
		s_stackId = 0;
	}

	void PopID()
	{
		ImGui::PopID();
		s_contextId--;
	}

	bool BeginProperties(const std::string& name, const ImVec2 size)
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

	void EndProperties()
	{
		ImGui::EndTable();
		ImGui::PopStyleColor(7);
		ImGui::PopStyleVar(2);
	}

	bool Property(const std::string& text, std::string& value, bool readOnly, const std::string& toolTip)
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

	bool Property(const std::string& text, std::filesystem::path& path, const std::string& toolTip)
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
			//auto newPath = FileSystem::OpenFileDialogue({ { "All (*.*)" }, { "*" } });
			//if (!newPath.empty())
			//{
			//	path = newPath;
			//	changed = true;
			//}
		}

		EndPropertyRow();

		return changed;
	}

	bool PropertyDirectory(const std::string& text, std::filesystem::path& path, const std::string& toolTip)
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

	void BeginPropertyRow()
	{
		auto* window = ImGui::GetCurrentWindow();
		window->DC.CurrLineSize.y = PROPERTY_ROW_HEIGHT;

		ImGui::TableNextRow(0, PROPERTY_ROW_HEIGHT);
		ImGui::TableNextColumn();
		window->DC.CurrLineTextBaseOffset = 3.f;

		SetPropertyBackgroundColor();
	}

	void EndPropertyRow()
	{
	}

	bool DrawItem(std::function<bool()> itemFunc)
	{
		const float itemWidth = ImGui::GetColumnWidth();

		ImGui::PushItemWidth(itemWidth);

		const bool itemHovered = IsItemHovered(itemWidth);

		if (itemHovered)
		{
			static const ImVec4 PropertyItemHovered = ImVec4{ 1.f, 1.f, 1.f, 1.f };
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

	bool DrawItem(const float itemWidth, std::function<bool()> itemFunc)
	{
		ImGui::PushItemWidth(itemWidth);

		const bool itemHovered = IsItemHovered(itemWidth);

		if (itemHovered)
		{
			static const ImVec4 PropertyItemHovered = ImVec4{ 1.f, 1.f, 1.f, 1.f };
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

	bool IsPropertyRowHovered()
	{
		const ImVec2 rowAreaMin = ImGui::TableGetCellBgRect(ImGui::GetCurrentTable(), 0).Min;
		const ImVec2 rowAreaMax = { ImGui::TableGetCellBgRect(ImGui::GetCurrentTable(), ImGui::TableGetColumnCount() - 1).Max.x, rowAreaMin.y + PROPERTY_ROW_HEIGHT + PROPERTY_ROW_PADDING * 2.f };

		ImGui::PushClipRect(rowAreaMin, rowAreaMax, false);
		const bool isRowHovered = ImGui::IsMouseHoveringRect(rowAreaMin, rowAreaMax, true);
		ImGui::PopClipRect();

		return isRowHovered;
	}

	bool IsPropertyColumnHovered(const uint32_t column)
	{
		const ImVec2 rowAreaMin = ImGui::TableGetCellBgRect(ImGui::GetCurrentTable(), static_cast<int32_t>(column)).Min;
		const ImVec2 rowAreaMax = { ImGui::TableGetCellBgRect(ImGui::GetCurrentTable(), static_cast<int32_t>(column)).Max.x, rowAreaMin.y + PROPERTY_ROW_HEIGHT + PROPERTY_ROW_PADDING * 2.f };

		ImGui::PushClipRect(rowAreaMin, rowAreaMax, false);
		const bool isColumnHovered = ImGui::IsMouseHoveringRect(rowAreaMin, rowAreaMax, true);
		ImGui::PopClipRect();

		return isColumnHovered;
	}

	void SetPropertyBackgroundColor()
	{
		static const ImVec4 PropertyBackground = { 36.f / 255.f, 36.f / 255.f, 36.f / 255.f, 1.f };
		static const ImVec4 PropertyBackgroundHovered = { 47.f / 255.f, 47.f / 255.f, 47.f / 255.f, 1.f };

		if (IsPropertyRowHovered())
		{
			SetRowColor(PropertyBackgroundHovered);
		}
		else
		{
			SetRowColor(PropertyBackground);
		}
	}

	void SetRowColor(const ImVec4& color)
	{
		for (int32_t i = 0; i < ImGui::TableGetColumnCount(); i++)
		{
			ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImColor{ color.x, color.y, color.z, color.w }, i);
		}
	}

	bool IsItemHovered(const float itemWidth)
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
}