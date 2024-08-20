#include "sbpch.h"
#include "EditorWindow.h"

#include "Sandbox/Utility/Theme.h"

EditorWindow::EditorWindow(const std::string& title, bool dockSpace, std::string id)
	: m_title(title + id), m_hasDockspace(dockSpace), m_id(id)
{
	m_backgroundColor = EditorTheme::DarkGreyBackground;

	BlockAllEvents();
}

bool EditorWindow::Begin()
{
	if (!m_isOpen && m_previousFrameOpen)
	{
		OnClose();
		m_previousFrameOpen = false;

		return false;
	}

	if (!m_isOpen)
	{
		return false;
	}

	if (m_hasDockspace)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0.f, 0.f });
	}

	bool minSize = false;
	if (m_minSize.x != -1.f || m_minSize.y != -1.f)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, m_minSize);
		minSize = true;
	}

	if (m_isFullscreenImage)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0.f, 0.f });
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0.f, 0.f });
	}

	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4{ m_backgroundColor.x, m_backgroundColor.y, m_backgroundColor.z, m_backgroundColor.w });

	m_previousFrameOpen = m_isOpen;
	ImGui::Begin(m_title.c_str(), &m_isOpen, m_windowFlags);

	if (minSize)
	{
		ImGui::PopStyleVar();
	}

	if (m_hasDockspace)
	{
		ImGui::PopStyleVar();

		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID(m_title.c_str());
			m_mainDockID = ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_AutoHideTabBar);

			m_windowClass.ClassId = dockspace_id;
			m_windowClass.DockingAllowUnclassed = false;
		}
	}

	m_isFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
	m_isHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows);
	m_isDockspace = ImGui::IsWindowDocked();

	return true;
}

void EditorWindow::End()
{
	ImGui::End();

	ImGui::PopStyleColor();

	if (m_isFullscreenImage)
	{
		ImGui::PopStyleVar(2);
	}
}

void EditorWindow::Open()
{
	UnblockAllEvents();

	m_isOpen = true;
	OnOpen();
}

void EditorWindow::Close()
{
	m_isOpen = false;
	OnClose();

	BlockAllEvents();
}

void EditorWindow::SetMinWindowSize(ImVec2 minSize)
{
	m_minSize = minSize;
}

void EditorWindow::Focus()
{
	auto window = ImGui::FindWindowByName(m_title.c_str());
	if (window)
	{
		ImGui::FocusWindow(window);
	}
}

bool EditorWindow::IsDocked() const
{
	return m_isDockspace;
}

void EditorWindow::ForceWindowDocked(ImGuiWindow* childWindow)
{
	if (m_mainDockID == 0)
	{
		return;
	}

	if (childWindow->DockNode && childWindow->DockNode->ParentNode)
	{
		m_dockIDs[childWindow->ID] = childWindow->DockNode->ParentNode->ID;
	}

	if (!childWindow->DockIsActive && !ImGui::IsAnyMouseDown() && !childWindow->DockNode && !childWindow->DockNodeIsVisible)
	{
		if (!m_dockIDs[childWindow->ID])
		{
			m_dockIDs[childWindow->ID] = m_mainDockID;
		}

		ImGui::SetWindowDock(childWindow, m_dockIDs[childWindow->ID], ImGuiCond_Always);
	}
}
