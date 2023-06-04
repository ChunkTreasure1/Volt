#include "sbpch.h"
#include "EditorSettingsPanel.h"

#include "Sandbox/Sandbox.h"
#include "Sandbox/VersionControl/VersionControl.h"
#include "Sandbox/UserSettingsManager.h"

#include <Volt/Core/Application.h>
#include <Volt/Utility/UIUtility.h>
#include <imgui_stdlib.h>

EditorSettingsPanel::EditorSettingsPanel(EditorSettings& settings)
	: EditorWindow("Editor Settings"), m_editorSettings(settings)
{
	myWindowFlags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
}

void EditorSettingsPanel::UpdateMainContent()
{
	const ImGuiTableFlags tableFlags = ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Resizable;

	if (ImGui::BeginTable("settingsMain", 2, tableFlags))
	{
		ImGui::TableSetupColumn("Outline", 0, 250.f);
		ImGui::TableSetupColumn("View", ImGuiTableColumnFlags_WidthStretch);

		ImGui::TableNextRow();
		ImGui::TableNextColumn();

		DrawOutline();

		ImGui::TableNextColumn();

		DrawView();

		ImGui::EndTable();
	}
}

void EditorSettingsPanel::DrawOutline()
{
	ImGuiStyle& style = ImGui::GetStyle();
	auto color = style.Colors[ImGuiCol_FrameBg];
	UI::ScopedColor newColor(ImGuiCol_ChildBg, { color.x, color.y, color.z, color.w });

	ImGui::BeginChild("##outline");

	UI::ShiftCursor(5.f, 5.f);
	if (ImGui::Selectable("Version Control"))
	{
		m_currentMenu = SettingsMenu::VersionControl;
	}
	UI::ShiftCursor(5.f, 5.f);
	if (ImGui::Selectable("External Tools"))
	{
		m_currentMenu = SettingsMenu::ExternalTools;
	}
	UI::ShiftCursor(5.f, 5.f);
	if (ImGui::Selectable("Style Settings"))
	{
		m_currentMenu = SettingsMenu::StyleSettings;
	}
	UI::ShiftCursor(5.f, 5.f);
	if (ImGui::Selectable("Editor Settings"))
	{
		m_currentMenu = SettingsMenu::EditorSettings;
	}

	ImGui::EndChild();
}

void EditorSettingsPanel::DrawView()
{
	ImGui::BeginChild("##view", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetWindowHeight()));
	{
		ImGui::BeginChild("scrolling");

		switch (m_currentMenu)
		{
			case SettingsMenu::VersionControl: DrawVersionControl(); break;
			case SettingsMenu::ExternalTools: DrawExternalTools(); break;
			case SettingsMenu::StyleSettings: DrawStyleSettings(); break;
			case SettingsMenu::EditorSettings: DrawEditorSettings(); break;
			default: break;
		}

		ImGui::EndChild();
	}
	ImGui::EndChild();
}

void EditorSettingsPanel::DrawVersionControl()
{
	auto& versionControlSettings = m_editorSettings.versionControlSettings;

	UI::PushId();
	if (UI::BeginProperties())
	{
		UI::Property("Host", versionControlSettings.server);
		UI::Property("User", versionControlSettings.user);
		UI::PropertyPassword("Password", versionControlSettings.password);

		UI::EndProperties();
	}

	{
		UI::ScopedColor buttonColor(ImGuiCol_Button, { 0.313f, 0.313f, 0.313f, 1.f });
		UI::ScopedStyleFloat buttonRounding(ImGuiStyleVar_FrameRounding, 2.f);

		if (ImGui::Button("Connect"))
		{
			if (!versionControlSettings.server.empty() &&
				!versionControlSettings.user.empty() && !versionControlSettings.password.empty())
			{
				if (VersionControl::Connect(m_editorSettings.versionControlSettings.server, m_editorSettings.versionControlSettings.user, m_editorSettings.versionControlSettings.password))
				{
					VersionControl::RefreshStreams();
					VersionControl::RefreshWorkspaces();
				}
			}
		}
	}

	ImGui::Separator();
	ImGui::TextUnformatted("Settings");

	if (VersionControl::IsConnected())
	{
		std::vector<std::string> streams = VersionControl::GetStreams();
		if (streams.empty())
		{
			streams.emplace_back("Empty");
		}

		std::vector<std::string> workspaces = VersionControl::GetWorkspaces();
		if (workspaces.empty())
		{
			workspaces.emplace_back("Empty");
		}

		UI::PushId();
		if (UI::BeginProperties())
		{
			int32_t currentStream = m_currentStream;
			int32_t currentWorkspace = m_currentWorkspace;

			if (UI::ComboProperty("Workspace", m_currentWorkspace, workspaces))
			{
				if (currentWorkspace != m_currentWorkspace)
				{
					UserSettingsManager::GetSettings().versionControlSettings.workspace = workspaces[m_currentWorkspace];

					VersionControl::SwitchWorkspace(workspaces[m_currentWorkspace]);
					VersionControl::RefreshStreams();
				}
			}

			//if (UI::ComboProperty("Stream", m_currentStream, streams))
			//{
			//	if (currentStream != m_currentStream)
			//	{
			//		VersionControl::SwitchStream(streams[m_currentStream]);
			//	}
			//}

			UI::EndProperties();
		}
		UI::PopId();

		ImGui::SameLine();
	}

	UI::PopId();
}

void EditorSettingsPanel::DrawExternalTools()
{
	auto& externalToolsSettings = m_editorSettings.externalToolsSettings;

	UI::PushId();
	if (UI::BeginProperties())
	{
		UI::Property("External Script Editor", externalToolsSettings.customExternalScriptEditor);

		UI::EndProperties();
	}
	UI::PopId();
}

void EditorSettingsPanel::DrawStyleSettings()
{
	float currentWindowOpacity = Volt::Application::Get().GetWindow().GetOpacity();

	UI::PushId();
	if (UI::BeginProperties())
	{
		if (UI::Property("Window Opacity", currentWindowOpacity, true, 0.f, 1.f))
		{
			Volt::Application::Get().GetWindow().SetOpacity(currentWindowOpacity);
		}
		UI::EndProperties();
	}
	UI::PopId();
}

void EditorSettingsPanel::DrawEditorSettings()
{
	auto& sceneSettings = m_editorSettings.sceneSettings;

	UI::PushId();
	if (UI::BeginProperties())
	{
		UI::Property("Low Memory Usage", sceneSettings.lowMemoryUsage);

		UI::EndProperties();
	}
	UI::PopId();
}
