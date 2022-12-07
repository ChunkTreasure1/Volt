#include "sbpch.h"
#include "LogPanel.h"

#include <Volt/Log/Log.h>
#include <Volt/Utility/UIUtility.h>

namespace Utility
{
	ImVec4 GetColorFromLevel(spdlog::level::level_enum aLevel)
	{
		switch (aLevel)
		{
			case spdlog::level::trace: return ImVec4(0.83f, 0.83f, 0.83f, 1.f);
			case spdlog::level::info: return ImVec4(1.f, 1.f, 1.f, 1.f);
			case spdlog::level::warn: return ImVec4(1.f, 0.92f, 0.21f, 1.f);
			case spdlog::level::err: return ImVec4(1.f, 0.f, 0.f, 1.f);
			case spdlog::level::critical: return ImVec4(1.f, 0.f, 0.f, 1.f);
		}

		return ImVec4(1.f, 1.f, 1.f, 1.f);
	}
}

LogPanel::LogPanel()
	: EditorWindow("Log")
{
	Volt::Log::AddCallback([&](const LogCallbackData& message) 
		{
			if (myLogMessages.size() >= myMaxMessages)
			{
				myLogMessages.erase(myLogMessages.begin());
			}

			myLogMessages.emplace_back(message);
		});
}

void LogPanel::UpdateMainContent()
{
	if (ImGui::Button("Clear"))
	{
		myLogMessages.clear();
	}

	ImGui::SameLine();
	static int32_t logLevel = 0;

	ImGui::PushItemWidth(200.f);
	if (ImGui::Combo("##level", &logLevel, "Trace\0Info\0Warning\0Error\0Critical"))
	{
		switch (logLevel)
		{
			case 0: Volt::Log::SetLogLevel(spdlog::level::trace); break;
			case 1: Volt::Log::SetLogLevel(spdlog::level::info); break;
			case 2: Volt::Log::SetLogLevel(spdlog::level::warn); break;
			case 3: Volt::Log::SetLogLevel(spdlog::level::err); break;
			case 4: Volt::Log::SetLogLevel(spdlog::level::critical); break;
		}
	}
	ImGui::PopItemWidth();

	UI::ScopedColor childColor(ImGuiCol_ChildBg, { 0.18f, 0.18f, 0.18f, 1.f });
	ImGui::BeginChild("log", ImGui::GetContentRegionAvail());
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.f, 0.f));

	for (const auto& msg : myLogMessages)
	{
		ImVec4 color = Utility::GetColorFromLevel(msg.level);
		ImGui::TextColored(color, msg.message.c_str());
	}

	if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
	{
		ImGui::SetScrollHere(1.f);
	}

	ImGui::PopStyleVar();
	ImGui::EndChild();
}
