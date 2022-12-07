#include "sbpch.h"
#include "ThemesPanel.h"

ThemesPanel::ThemesPanel() : EditorWindow("Themes") 
{
	myIo = &ImGui::GetIO();
}

void ThemesPanel::UpdateMainContent()
{
	if(ImGui::Button("Minecraft"))
	{
		SetMinecraftTheme();
	}
}

void ThemesPanel::SetMinecraftTheme()
{
	ImGuiStyle& style = ImGui::GetStyle();
	if (myIo->ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.f;
	}

	style.Colors[ImGuiCol_Text] = ImVec4(1.000f, 1.000f, 1.000f, 1.000f);
	style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.500f, 0.500f, 0.500f, 1.000f);

	style.Colors[ImGuiCol_WindowBg] = ImVec4(155.f / 255.f, 118.f / 255.f, 83.f / 255.f, 1.000f);
	style.Colors[ImGuiCol_ChildBg] = ImVec4(0.280f, 0.280f, 0.280f, 0.000f);
	style.Colors[ImGuiCol_PopupBg] = ImVec4(0.313f, 0.313f, 0.313f, 1.000f);

	style.Colors[ImGuiCol_Border] = ImVec4(0.137f, 0.137f, 0.137f, 1.000f);
	style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.000f, 0.000f, 0.000f, 0.000f);

	style.Colors[ImGuiCol_FrameBg] = ImVec4(0.160f, 0.160f, 0.160f, 1.000f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.200f, 0.200f, 0.200f, 1.000f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.280f, 0.280f, 0.280f, 1.000f);

	style.Colors[ImGuiCol_TitleBg] = ImVec4(94.f / 255.f, 157.f / 255.f, 52.f / 255.f, 1.000f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.137f, 0.137f, 0.137f, 1.000f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.148f, 0.148f, 0.148f, 1.000f);

	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.195f, 0.195f, 0.195f, 1.000f);

	style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.160f, 0.160f, 0.160f, 1.000f);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.277f, 0.277f, 0.277f, 1.000f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.300f, 0.300f, 0.300f, 1.000f);
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.4f, 0.67f, 1.000f, 1.000f);

	style.Colors[ImGuiCol_CheckMark] = ImVec4(1.000f, 1.000f, 1.000f, 1.000f);

	style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.391f, 0.391f, 0.391f, 1.000f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.4f, 0.67f, 1.000f, 1.000f);

	style.Colors[ImGuiCol_Button] = ImVec4(0.258f, 0.258f, 0.258f, 1.000f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(1.000f, 1.000f, 1.000f, 0.156f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(1.000f, 1.000f, 1.000f, 0.391f);


	style.Colors[ImGuiCol_Header] = ImVec4(0.313f, 0.313f, 0.313f, 1.000f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.469f, 0.469f, 0.469f, 1.000f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.469f, 0.469f, 0.469f, 1.000f);

	style.Colors[ImGuiCol_Separator] = style.Colors[ImGuiCol_Border];
	style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.391f, 0.391f, 0.391f, 1.000f);
	style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.4f, 0.67f, 1.000f, 1.000f);

	style.Colors[ImGuiCol_ResizeGrip] = ImVec4(1.000f, 1.000f, 1.000f, 0.250f);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.000f, 1.000f, 1.000f, 0.670f);
	style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.4f, 0.67f, 1.000f, 1.000f);

	style.Colors[ImGuiCol_Tab] = ImVec4(0.137f, 0.137f, 0.137f, 1.000f);
	style.Colors[ImGuiCol_TabTop] = ImVec4(0.4f, 0.67f, 1.000f, 1.000f);
	style.Colors[ImGuiCol_TabHovered] = ImVec4(0.352f, 0.352f, 0.352f, 1.000f);
	style.Colors[ImGuiCol_TabActive] = ImVec4(0.258f, 0.258f, 0.258f, 1.000f);
	style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.137f, 0.137f, 0.137f, 1.000f);
	style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.258f, 0.258f, 0.258f, 1.000f);

	style.Colors[ImGuiCol_DockingPreview] = ImVec4(0.4f, 0.67f, 1.000f, 0.781f);
	style.Colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.137f, 0.137f, 0.137f, 1.000f);

	style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.160f, 0.160f, 0.160f, 1.000f);
	style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.160f, 0.160f, 0.160f, 1.000f);
	style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.160f, 0.160f, 0.160f, 1.000f);

	style.Colors[ImGuiCol_PlotLines] = ImVec4(0.469f, 0.469f, 0.469f, 1.000f);
	style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.4f, 0.67f, 1.000f, 1.000f);
	style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.586f, 0.586f, 0.586f, 1.000f);
	style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);

	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(1.000f, 1.000f, 1.000f, 0.156f);
	style.Colors[ImGuiCol_DragDropTarget] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);

	style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.4f, 0.67f, 1.000f, 1.000f);
	style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.4f, 0.67f, 1.000f, 1.000f);
	style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.000f, 0.000f, 0.000f, 0.586f);
	style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.000f, 0.000f, 0.000f, 0.586f);

	style.ChildRounding = 0;
	style.FrameRounding = 0;
	style.GrabMinSize = 7.0f;
	style.PopupRounding = 2.0f;
	style.ScrollbarRounding = 12.0f;
	style.ScrollbarSize = 13.0f;
	style.TabBorderSize = 0.0f;
	style.TabRounding = 0.0f;
	style.WindowRounding = 0.0f;
	style.WindowBorderSize = 2.f;
}
