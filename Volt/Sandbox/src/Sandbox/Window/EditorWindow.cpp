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

	bool minSize = false;
	if (myMinSize.x != -1.f || myMinSize.y != -1.f)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, myMinSize);
		minSize = true;
	}

	ImGui::Begin(myTitle.c_str(), &myIsOpen, myWindowFlags);

	if (minSize)
	{
		ImGui::PopStyleVar();
	}

	if (myHasDockSpace)
	{
		ImGui::PopStyleVar();

		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID(myTitle.c_str());
			myMainDockId = ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
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

void EditorWindow::ForceWindowDocked(ImGuiWindow* childWindow)
{
	if (myMainDockId == 0)
	{
		return;
	}

	if (childWindow->DockNode && childWindow->DockNode->ParentNode)
	{
		myDockIds[childWindow->ID] = childWindow->DockNode->ParentNode->ID;
	}
		
	if (!childWindow->DockIsActive && !ImGui::IsAnyMouseDown() && !childWindow->DockNode && !childWindow->DockNodeIsVisible)
	{
		if (!myDockIds[childWindow->ID])
		{
			myDockIds[childWindow->ID] = myMainDockId;
		}

		ImGui::SetWindowDock(childWindow, myDockIds[childWindow->ID], ImGuiCond_Always);
	}
}