#include "sbpch.h"
#include "ProjectUpgradeLayer.h"

#include "Sandbox/Utility/Theme.h"
#include "ProjectUpgrade/V011/V011Convert.h"
#include "ProjectUpgrade/V012/V012Convert.h"
#include "ProjectUpgrade/V013/V013Convert.h"
#include "ProjectUpgrade/V015/V015Convert.h"

#include <Volt/Core/Application.h>
#include <Volt/Events/ApplicationEvents.h>
#include <Volt/Utility/UIUtility.h>

#include <WindowModule/Events/WindowEvents.h>
#include <WindowModule/WindowManager.h>
#include <WindowModule/Window.h>

#include <imgui.h>

ProjectUpgradeLayer::ProjectUpgradeLayer()
{
}

ProjectUpgradeLayer::~ProjectUpgradeLayer()
{
}

void ProjectUpgradeLayer::OnAttach()
{
}

void ProjectUpgradeLayer::OnDetach()
{
}

void ProjectUpgradeLayer::OnEvent(Volt::Event& e)
{
	Volt::EventDispatcher dispatcher{ e };
	dispatcher.Dispatch<Volt::AppImGuiUpdateEvent>(VT_BIND_EVENT_FN(ProjectUpgradeLayer::OnImGuiUpdateEvent));
}

void ProjectUpgradeLayer::DrawUpgradeUI()
{
	ImGui::Text("The current project is not the same version as the engine!");
	ImGui::Text("Would you like to upgrade?");

	const Volt::Version engineVersion = Volt::Application::Get().GetInfo().version;
	const Volt::Version projectVersion = Volt::ProjectManager::GetProject().engineVersion;

	if (ImGui::Button("Yes"))
	{
		if (projectVersion.GetMinor() < 1 && projectVersion.GetMajor() == 0)
		{
			V011::Convert();
		}

		if (projectVersion.GetPatch() < 2 && projectVersion.GetMinor() < 2 && projectVersion.GetMajor() == 0)
		{
			V012::Convert();
		}

		if (projectVersion.GetPatch() < 3 && projectVersion.GetMinor() < 2 && projectVersion.GetMajor() == 0)
		{
			V013::Convert();
		}

		if (projectVersion.GetPatch() < 5 && projectVersion.GetMinor() < 2 && projectVersion.GetMajor() == 0)
		{
			V015::Convert();
		}

		Volt::ProjectManager::OnProjectUpgraded();
		Volt::ProjectManager::SerializeProject();

		Volt::WindowCloseEvent loadEvent{};
		Volt::Application::Get().OnEvent(loadEvent);
	}

	ImGui::SameLine();

	if (ImGui::Button("No"))
	{
		Volt::WindowCloseEvent loadEvent{};
		Volt::Application::Get().OnEvent(loadEvent);
	}
}

bool ProjectUpgradeLayer::OnImGuiUpdateEvent(Volt::AppImGuiUpdateEvent& e)
{
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigWindowsResizeFromEdges = io.BackendFlags & ImGuiBackendFlags_HasMouseCursors;

	// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
	// because it would be confusing to have two docking targets within each others.
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;

	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

	const bool isMaximized = Volt::WindowManager::Get().GetMainWindow().IsMaximized();

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, isMaximized ? ImVec2(6.0f, 6.0f) : ImVec2(1.0f, 1.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 3.0f);
	ImGui::PushStyleColor(ImGuiCol_MenuBarBg, ImVec4{ 0.0f, 0.0f, 0.0f, 0.0f });
	ImGui::Begin("MainDockspace", nullptr, window_flags);
	ImGui::PopStyleColor(); // MenuBarBg
	ImGui::PopStyleVar(2);
	ImGui::PopStyleVar(2);

	DrawUpgradeUI();

	ImGui::End();

	return false;
}
