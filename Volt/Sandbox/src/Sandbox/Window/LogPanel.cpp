#include "sbpch.h"
#include "LogPanel.h"

#include <Volt/Log/Log.h>
#include <Volt/Utility/UIUtility.h>
#include <Volt/Utility/StringUtility.h>

#include <Volt/Console/ConsoleVariableRegistry.h>

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
	m_isOpen = true;

	Volt::Log::AddCallback([&](const LogCallbackData& message)
	{
		if (myLogMessages.size() >= myMaxMessages)
		{
			myLogMessages.erase(myLogMessages.begin());
		}

		std::scoped_lock lock{ m_logMutex };
		myLogMessages.emplace_back(message);
	});

	myCategories.emplace_back("Default");
}

void LogPanel::UpdateMainContent()
{
	std::scoped_lock lock{ m_logMutex };

	if (ImGui::Button("Clear"))
	{
		myLogMessages.clear();
		myCategories.clear();
		myCategories.emplace_back("Default");
	}

	ImGui::SameLine();
	ImGui::PushItemWidth(200.f);

	static std::string commandStr;

	if (ImGui::InputTextWithHintString("##commandLine", "Command...", &commandStr, ImGuiInputTextFlags_EnterReturnsTrue))
	{
		auto strings = Utility::SplitStringsByCharacter(commandStr, ' ');
		if (!strings.empty())
		{
			if (Volt::ConsoleVariableRegistry::VariableExists(strings[0]))
			{
				auto variable = Volt::ConsoleVariableRegistry::GetVariable(strings[0]);

				std::string message = std::string(variable->GetName()) + " = ";

				if (strings.size() > 1)
				{
					if (variable->IsFloat())
					{
						const float value = std::stof(strings[1]);
						variable->Set(&value);
					}
					else if (variable->IsInteger())
					{
						const int32_t value = std::stoi(strings[1]);
						variable->Set(&value);
					}
					else if (variable->IsString())
					{
						variable->Set(&strings[1]);
					}

					message += strings[1];
				}
				else
				{
					if (variable->IsFloat())
					{
						message += std::to_string(*static_cast<const float*>(variable->Get()));
					}
					else if (variable->IsInteger())
					{
						message += std::to_string(*static_cast<const int32_t*>(variable->Get()));
					}
					else if (variable->IsString())
					{
						message += *static_cast<const std::string*>(variable->Get());
					}

				}

				VT_CORE_TRACE(message);
			}
			else
			{
				VT_CORE_TRACE("Command {0} not found!", strings[0]);
			}
		}

		commandStr = "";
	}


	ImGui::SameLine();
	static int32_t logLevel = 0;

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

	ImGui::SameLine();
	static int32_t logCategory = 0;

	if (logCategory > myCategories.size() - 1)
	{
		logCategory = 0;
	}

	ImGui::PushItemWidth(200.f);
	UI::Combo("Category", *(int*)&logCategory, myCategories);
	ImGui::PopItemWidth();

	myCurrentLogMessages.clear();
	for (const auto msg : myLogMessages)
	{
		bool newCategory = true;

		for (const auto& cat : myCategories)
		{
			if (cat == msg.category)
			{
				newCategory = false;
				break;
			}
		}

		if (newCategory)
		{
			myCategories.emplace_back(msg.category);
		}

		if (msg.level >= logLevel && (logCategory == 0 || msg.category == myCategories[logCategory]))
		{
			myCurrentLogMessages.emplace_back(msg);
		}
	}

	UI::ScopedColor childColor(ImGuiCol_ChildBg, { 0.18f, 0.18f, 0.18f, 1.f });
	ImGui::BeginChild("log", ImGui::GetContentRegionAvail());
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.f, 0.f));

	for (const auto& msg : myCurrentLogMessages)
	{
		ImVec4 color = Utility::GetColorFromLevel(msg.level);

		ImGui::PushStyleColor(ImGuiCol_Text, Utility::GetColorFromLevel(msg.level));
		ImGui::TextWrapped(msg.message.c_str());
		ImGui::PopStyleColor();
	}

	if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
	{
		ImGui::SetScrollHereY(1.f);
	}

	ImGui::PopStyleVar();
	ImGui::EndChild();
}
