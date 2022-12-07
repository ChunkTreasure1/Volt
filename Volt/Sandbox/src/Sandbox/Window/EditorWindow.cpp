#include "sbpch.h"
#include "EditorWindow.h"

EditorWindow::EditorWindow(const std::string& title, bool dockSpace)
	: myTitle(title), myHasDockSpace(dockSpace)
{}

bool EditorWindow::Begin()
{
	if (!myIsOpen)
	{
		return false;
	}

	if (myHasDockSpace)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0.f, 0.f });
	}

	ImGui::Begin(myTitle.c_str(), &myIsOpen, myWindowFlags);

	if (myHasDockSpace)
	{
		ImGui::PopStyleVar();

		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID(myTitle.c_str());
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
		}
	}

	myIsFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
	myIsHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows);

	return true;
}

void EditorWindow::End()
{
	ImGui::End();
}

void EditorWindow::Open()
{
	myIsOpen = true;
}

void EditorWindow::Close()
{
	myIsOpen = false;
}

void EditorWindow::SetMinWindowSize(ImVec2 minSize)
{
	myMinSize = minSize;
}
