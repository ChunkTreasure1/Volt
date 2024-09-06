#include "sbpch.h"
#include "Window/LogPanel.h"

#include <Volt/Utility/UIUtility.h>
#include <Volt/Utility/StringUtility.h>

#include <Volt/Console/ConsoleVariableRegistry.h>

namespace Utility
{
	ImVec4 GetColorFromLevel(LogVerbosity aLevel)
	{
		switch (aLevel)
		{
			case LogVerbosity::Trace: return ImVec4(0.83f, 0.83f, 0.83f, 1.f);
			case LogVerbosity::Info: return ImVec4(1.f, 1.f, 1.f, 1.f);
			case LogVerbosity::Warning: return ImVec4(1.f, 0.92f, 0.21f, 1.f);
			case LogVerbosity::Error: return ImVec4(1.f, 0.f, 0.f, 1.f);
			case LogVerbosity::Critical: return ImVec4(1.f, 0.f, 0.f, 1.f);
		}

		return ImVec4(1.f, 1.f, 1.f, 1.f);
	}
}

LogPanel::LogPanel()
	: EditorWindow("Log")
{
	Open();

	m_callbackHandle = Log::Get().RegisterCallback([&](const LogCallbackData& message)
	{
		//std::scoped_lock lock{ m_logMutex };
		m_logMessages.emplace_back(message);
		if (m_logMessages.size() >= m_maxMessages)
		{
			m_logMessages.erase(m_logMessages.begin());
		}
	});

	m_categories.emplace_back("Default");
}

LogPanel::~LogPanel()
{
	Log::Get().UnregisterCallback(m_callbackHandle);
	m_callbackHandle = 0;
}

void LogPanel::UpdateMainContent()
{
	if (ImGui::Button("Clear"))
	{
		std::scoped_lock lock{ m_logMutex };

		m_logMessages.clear();
		m_categories.clear();
		m_categories.emplace_back("Default");
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

				VT_LOG(Trace, message);
			}
			else
			{
				VT_LOG(Trace, "Command {0} not found!", strings[0]);
			}
		}

		commandStr = "";
	}

	std::scoped_lock lock{ m_logMutex };

	ImGui::SameLine();
	static int32_t logLevel = 0;

	if (ImGui::Combo("##level", &logLevel, "Trace\0Info\0Warning\0Error\0Critical"))
	{
	}
	ImGui::PopItemWidth();

	ImGui::SameLine();
	static int32_t logCategory = 0;

	if (logCategory > m_categories.size() - 1)
	{
		logCategory = 0;
	}

	ImGui::PushItemWidth(200.f);
	UI::Combo("Category", *(int*)&logCategory, m_categories);
	ImGui::PopItemWidth();

	m_currentLogMessages.clear();
	for (const auto msg : m_logMessages)
	{
		bool newCategory = true;

		for (const auto& cat : m_categories)
		{
			if (cat == msg.category)
			{
				newCategory = false;
				break;
			}
		}

		if (newCategory)
		{
			m_categories.emplace_back(msg.category);
		}

		if (static_cast<int32_t>(msg.severity) >= logLevel && (logCategory == 0 || msg.category == m_categories[logCategory]))
		{
			m_currentLogMessages.emplace_back(msg);
		}
	}

	UI::ScopedColor childColor(ImGuiCol_ChildBg, { 0.18f, 0.18f, 0.18f, 1.f });
	ImGui::BeginChild("log", ImGui::GetContentRegionAvail());
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.f, 0.f));

	for (const auto& msg : m_currentLogMessages)
	{
		ImVec4 color = Utility::GetColorFromLevel(msg.severity);

		ImGui::PushStyleColor(ImGuiCol_Text, Utility::GetColorFromLevel(msg.severity));
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
