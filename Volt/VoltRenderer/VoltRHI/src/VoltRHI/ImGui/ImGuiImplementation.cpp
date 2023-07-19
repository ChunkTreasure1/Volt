#include "rhipch.h"
#include "ImGuiImplementation.h"

#include "VoltRHI/Graphics/GraphicsContext.h"
#include "VoltRHI/Core/Core.h"

#include <VoltVulkan/ImGui/VulkanImGuiImplementation.h>

#include <imgui.h>
//#include <imgui_notify.h>

#include <tahoma.h>
#include "FontAwesome.h"

namespace Volt
{
	//void MergeIconsWithLatestFont(float font_size)
	//{
	//	ImGuiIO& io = ImGui::GetIO();

	//	float baseFontSize = font_size; // 13.0f is the size of the default font. Change to the font size you use.
	//	float iconFontSize = baseFontSize; // FontAwesome fonts need to have their sizes reduced by 2.0f/3.0f in order to align correctly

	//	// merge in icons from Font Awesome
	//	static const ImWchar icons_ranges[] = { ICON_MIN_FA, VT_ICON_MAX_16_FA, 0 };
	//	ImFontConfig icons_config;
	//	icons_config.MergeMode = true;
	//	icons_config.PixelSnapH = true;
	//	icons_config.GlyphMinAdvanceX = iconFontSize;
	//	io.Fonts->AddFontFromFileTTF("Engine/Fonts/FontAwesome/" FONT_ICON_FILE_NAME_FAS, iconFontSize, &icons_config, icons_ranges);
	//}

	std::filesystem::path GetOrCreateIniPath()
	{
		const std::filesystem::path userIniPath = "User/imgui.ini";
		const std::filesystem::path defaultIniPath = "Editor/imgui.ini";

		if (!std::filesystem::exists(userIniPath))
		{
			GraphicsContext::Log(Severity::Warning, "[ImGui] User ini file not found! Copying default!");

			std::filesystem::create_directories(userIniPath.parent_path());
			if (!std::filesystem::exists(defaultIniPath))
			{
				GraphicsContext::Log(Severity::Error, "[ImGui] Unable to find default ini file!");
				return "imgui.ini";
			}
			std::filesystem::copy(defaultIniPath, userIniPath.parent_path());
		}

		return userIniPath;
	}

	ImGuiImplementation::ImGuiImplementation()
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
		io.ConfigWindowsMoveFromTitleBarOnly = true;

		ImFontConfig fontCfg;
		fontCfg.FontDataOwnedByAtlas = true;
		//UI::SetFont(FontType::Regular_16, io.Fonts->AddFontFromFileTTF("Engine/Fonts/Inter/inter-Regular.ttf", 16.f));
		//MergeIconsWithLatestFont(16.f);

		//UI::SetFont(FontType::Regular_17, io.Fonts->AddFontFromFileTTF("Engine/Fonts/Inter/inter-Regular.ttf", 17.f));
		//MergeIconsWithLatestFont(17.f);

		//UI::SetFont(FontType::Regular_20, io.Fonts->AddFontFromFileTTF("Engine/Fonts/Inter/inter-Regular.ttf", 20.f));
		//MergeIconsWithLatestFont(20.f);

		//UI::SetFont(FontType::Bold_17, io.Fonts->AddFontFromFileTTF("Engine/Fonts/Inter/inter-Bold.ttf", 17.f));
		//MergeIconsWithLatestFont(17.f);

		//UI::SetFont(FontType::Bold_16, io.Fonts->AddFontFromFileTTF("Engine/Fonts/Inter/inter-Bold.ttf", 16.f));
		//MergeIconsWithLatestFont(16.f);

		//UI::SetFont(FontType::Bold_90, io.Fonts->AddFontFromFileTTF("Engine/Fonts/Inter/inter-Bold.ttf", 90.f));
		//MergeIconsWithLatestFont(90.f);

		//UI::SetFont(FontType::Bold_20, io.Fonts->AddFontFromFileTTF("Engine/Fonts/Inter/inter-Bold.ttf", 20.f));
		//MergeIconsWithLatestFont(20.f);

		//UI::SetFont(FontType::Bold_12, io.Fonts->AddFontFromFileTTF("Engine/Fonts/Inter/inter-Bold.ttf", 12.f));
		//MergeIconsWithLatestFont(12.f);

		//UI::SetFont(FontType::Regular_12, io.Fonts->AddFontFromFileTTF("Engine/Fonts/Inter/inter-Regular.ttf", 12.f));
		//MergeIconsWithLatestFont(12.f);

		//fontCfg.FontDataOwnedByAtlas = false;
		//io.Fonts->AddFontFromMemoryTTF((void*)tahoma, sizeof(tahoma), 17.f, &fontCfg);
		//ImGui::MergeIconsWithLatestFont(17.f, false);

		io.IniFilename = nullptr;

		const std::filesystem::path iniPath = GetOrCreateIniPath();
		ImGui::LoadIniSettingsFromDisk(iniPath.string().c_str());

		ImGui::StyleColorsDark();

		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.f;
		}

		style.Colors[ImGuiCol_Text] = ImVec4(1.000f, 1.000f, 1.000f, 1.000f);
		style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.500f, 0.500f, 0.500f, 1.000f);

		style.Colors[ImGuiCol_WindowBg] = ImVec4(0.23f, 0.23f, 0.23f, 1.000f);
		style.Colors[ImGuiCol_ChildBg] = ImVec4(0.280f, 0.280f, 0.280f, 0.000f);
		style.Colors[ImGuiCol_PopupBg] = ImVec4(0.313f, 0.313f, 0.313f, 1.000f);

		style.Colors[ImGuiCol_Border] = ImVec4(0.137f, 0.137f, 0.137f, 1.000f);
		style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.000f, 0.000f, 0.000f, 0.000f);

		style.Colors[ImGuiCol_FrameBg] = ImVec4(0.160f, 0.160f, 0.160f, 1.000f);
		style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.200f, 0.200f, 0.200f, 1.000f);
		style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.280f, 0.280f, 0.280f, 1.000f);

		style.Colors[ImGuiCol_TitleBg] = ImVec4(0.137f, 0.137f, 0.137f, 1.000f);
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
		style.Colors[ImGuiCol_DragDropTarget] = ImVec4(0.4f, 0.67f, 1.000f, 1.000f);

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

		InitializeAPI();
	}

	ImGuiImplementation::~ImGuiImplementation()
	{
		ShutdownAPI();

		const std::filesystem::path iniPath = GetOrCreateIniPath();
		ImGui::SaveIniSettingsToDisk(iniPath.string().c_str());
		ImGui::DestroyContext();
	}

	void ImGuiImplementation::Begin()
	{
		BeginAPI();
		ImGui::NewFrame();
	}

	void ImGuiImplementation::End()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 5.f); // Round borders
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(43.f / 255.f, 43.f / 255.f, 43.f / 255.f, 100.f / 255.f)); // Background color
		//ImGui::RenderNotifications(); // <-- Here we render all notifications
		ImGui::PopStyleVar(1); // Don't forget to Pop()
		ImGui::PopStyleColor(1);

		//Rendering
		ImGui::Render();

		EndAPI();

		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
	}

	Ref<ImGuiImplementation> ImGuiImplementation::Create(const ImGuiCreateInfo& createInfo)
	{
		const auto api = GraphicsContext::GetAPI();

		switch (api)
		{
			case GraphicsAPI::D3D12:
			case GraphicsAPI::MoltenVk:
			case GraphicsAPI::Mock:
				break;
			
			case GraphicsAPI::Vulkan: return CreateRefRHI<VulkanImGuiImplementation>(createInfo);
		}

		return nullptr;
	}
}
