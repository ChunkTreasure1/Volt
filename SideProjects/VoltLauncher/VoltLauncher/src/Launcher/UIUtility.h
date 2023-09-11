#pragma once

#include <imgui.h>
#include <string>
#include <filesystem>
#include <functional>

namespace UI
{
	void SimpleToolTip(const std::string& toolTip);
	bool InputText(const std::string& name, std::string& text, ImGuiInputTextFlags_ flags = ImGuiInputTextFlags_None);
	bool InputTextString(const char* label, std::string* str, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr, void* user_data = nullptr);

	void PushID();
	void PopID();

	bool BeginProperties(const std::string& name = "", const ImVec2 size = { 0, 0 });
	void EndProperties();

	bool Property(const std::string& text, std::string& value, bool readOnly = false, const std::string& toolTip = "");
	bool Property(const std::string& text, std::filesystem::path& path, const std::string& toolTip = "");
	bool PropertyDirectory(const std::string& text, std::filesystem::path& path, const std::string& toolTip = "");

	void BeginPropertyRow();
	void EndPropertyRow();

	bool DrawItem(std::function<bool()> itemFunc);
	bool DrawItem(const float itemWidth, std::function<bool()> itemFunc);

	bool IsPropertyRowHovered();
	bool IsPropertyColumnHovered(const uint32_t column);
	void SetPropertyBackgroundColor();
	void SetRowColor(const ImVec4& color);
	
	bool IsItemHovered(const float itemWidth);
}