#include "LauncherLayer.h"

#include "FileSystem.h"

#include "Walnut/Image.h"
#include "Walnut/UI/UI.h"

inline static ImVec4 ToNormalizedRGB(float r, float g, float b, float a = 255.f)
{
	return ImVec4{ r / 255.f, g / 255.f, b / 255.f, a / 255.f };
}

void LauncherLayer::OnAttach()
{
	if (FileSystem::HasEnvironmentVariable("VOLT_PATH"))
	{
		const std::filesystem::path engineDir = FileSystem::GetEnvVariable("VOLT_PATH");
		const bool hasSandboxExe = std::filesystem::exists(engineDir / "Sandbox.exe");
		
		if (hasSandboxExe)
		{
			m_data.engineInfo.engineDirectory = engineDir;
		}
	}
}

void LauncherLayer::OnUIRender()
{
	const ImGuiTableFlags flags = ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingFixedFit;

	if (ImGui::BeginTable("mainTable", 2, flags))
	{
		constexpr float TABS_WIDTH = 300.f;

		ImGui::TableSetupColumn("Column1", 0, TABS_WIDTH);
		ImGui::TableSetupColumn("Column2", 0, ImGui::GetContentRegionAvail().x - TABS_WIDTH);

		ImGui::TableNextRow();
		ImGui::TableNextColumn();

		UI_DrawTabsChild();

		ImGui::TableNextColumn();

		UI_DrawContentChild();

		ImGui::EndTable();
	}

	UI_DrawAboutModal();
}

void LauncherLayer::UI_DrawAboutModal()
{
	if (!m_aboutModalOpen)
		return;

	ImGui::OpenPopup("About");
	m_aboutModalOpen = ImGui::BeginPopupModal("About", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
	if (m_aboutModalOpen)
	{
		auto image = Walnut::Application::Get().GetApplicationIcon();
		ImGui::Image(image->GetDescriptorSet(), { 48, 48 });

		ImGui::SameLine();
		Walnut::UI::ShiftCursorX(20.0f);

		ImGui::BeginGroup();
		ImGui::Text("Walnut application framework");
		ImGui::Text("by Studio Cherno.");
		ImGui::EndGroup();

		if (Walnut::UI::ButtonCentered("Close"))
		{
			m_aboutModalOpen = false;
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}

void LauncherLayer::UI_DrawTabsChild()
{
	if (ImGui::BeginChild("Tabs"))
	{
		{
			Walnut::UI::ShiftCursorY(10.f);

			Walnut::UI::ScopedColorStack buttonColor{ ImGuiCol_Button, ImVec4{ 0.f, 0.f, 0.f, 0.f }, ImGuiCol_ButtonHovered, ToNormalizedRGB(14.f, 134.f, 225.f), ImGuiCol_ButtonActive, ToNormalizedRGB(0.f, 80.f, 160.f) };
			Walnut::UI::ScopedStyle frameRounding{ ImGuiStyleVar_FrameRounding, 0.f };
			Walnut::UI::ScopedStyle itemSpacing{ ImGuiStyleVar_ItemSpacing, ImVec2{ 0.f, 0.f } };
			Walnut::UI::ScopedStyle borderSize{ ImGuiStyleVar_FrameBorderSize, 0.f };

			if (ImGui::Button("Projects", ImVec2{ ImGui::GetContentRegionAvail().x, 50.f }))
			{
				m_currentTab = Tab::Projects;
			}

			if (ImGui::Button("Engines", ImVec2{ ImGui::GetContentRegionAvail().x, 50.f }))
			{
				m_currentTab = Tab::Engines;
			}
		}

		ImGui::EndChild();
	}
}

void LauncherLayer::UI_DrawContentChild()
{
	if (ImGui::BeginChild("Content"))
	{
		if (m_currentTab == Tab::Projects)
		{
			UI_DrawProjectsContent();
		}
		else if (m_currentTab == Tab::Engines)
		{
			UI_DrawEnginesContent();
		}

		ImGui::EndChild();
	}
}

void LauncherLayer::UI_DrawProjectsContent()
{
	Walnut::UI::ShiftCursor(40.f, 50.f);

	{
		Walnut::UI::ScopedFont font{ Walnut::Application::GetFont("BoldHeader") };
		ImGui::TextUnformatted("Projects");
	}

	ImGui::SameLine(ImGui::GetContentRegionAvail().x - 200.f);

	if (ImGui::Button("Locate"))
	{
	}

	ImGui::SameLine();

	if (ImGui::Button("Add"))
	{

	}

	if (ImGui::BeginChild("ProjectsChild", ImGui::GetContentRegionAvail()))
	{


		ImGui::EndChild();
	}
}

void LauncherLayer::UI_DrawEnginesContent()
{
	Walnut::UI::ShiftCursor(40.f, 50.f);

	{
		Walnut::UI::ScopedFont font{ Walnut::Application::GetFont("BoldHeader") };
		ImGui::TextUnformatted("Engine");
	}

	ImGui::SameLine(ImGui::GetContentRegionAvail().x - 200.f);

	if (ImGui::Button("Locate"))
	{
	}

	ImGui::SameLine();

	if (ImGui::Button("Add"))
	{

	}

	if (ImGui::BeginChild("EngineChild", ImGui::GetContentRegionAvail()))
	{
		if (!m_data.engineInfo.IsValid())
		{
			ImGui::TextUnformatted("No engine has been found!");
		}

		ImGui::EndChild();
	}
}

void LauncherLayer::ShowAboutModal()
{
	m_aboutModalOpen = true;
}
