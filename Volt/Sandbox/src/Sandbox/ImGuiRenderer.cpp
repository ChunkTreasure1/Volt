#include "sbpch.h"
#include "Sandbox.h"

#include "Sandbox/Window/EditorWindow.h"
#include "Sandbox/Window/AssetBrowser/AssetBrowserPanel.h"
#include "Sandbox/Utility/EditorIconLibrary.h"
#include "Sandbox/Utility/EditorUtilities.h"

#include <Volt/Scripting/Mono/MonoScriptEngine.h>
#include <Volt/Core/Application.h>
#include <Volt/Asset/AssetManager.h>
#include <Volt/Rendering/Shader/ShaderRegistry.h>
#include <Volt/Rendering/Shader/Shader.h>

#include <Volt/Utility/FileSystem.h>
#include <Volt/Utility/UIUtility.h>

#include <imgui.h>

void Sandbox::UpdateDockSpace()
{
	ImGuiIO& io = ImGui::GetIO();
	ImGuiStyle& style = ImGui::GetStyle();
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

	const bool isMaximized = Volt::Application::Get().GetWindow().IsMaximized();

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, isMaximized ? ImVec2(6.0f, 6.0f) : ImVec2(1.0f, 1.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 3.0f);
	ImGui::PushStyleColor(ImGuiCol_MenuBarBg, ImVec4{ 0.0f, 0.0f, 0.0f, 0.0f });
	ImGui::Begin("MainDockspace", nullptr, window_flags);
	ImGui::PopStyleColor(); // MenuBarBg
	ImGui::PopStyleVar(2);

	ImGui::PopStyleVar(2);

	{
		UI::ScopedColor windowBorder(ImGuiCol_Border, IM_COL32(50, 50, 50, 255));
		// Draw window border if the window is not maximized
		if (!isMaximized)
			RenderWindowOuterBorders(ImGui::GetCurrentWindow());
	}

	HandleManualWindowResize();

	const float titlebarHeight = DrawTitlebar();
	ImGui::SetCursorPosY(titlebarHeight + ImGui::GetCurrentWindow()->WindowPadding.y);

	// Dockspace
	ImGui::DockSpace(ImGui::GetID("MainDockspace"));
	style.WindowMinSize.x = 200.f;

	ImGui::End();
}

void Sandbox::RenderWindowOuterBorders(ImGuiWindow* window)
{
	struct ImGuiResizeBorderDef
	{
		ImVec2 InnerDir;
		ImVec2 SegmentN1, SegmentN2;
		float  OuterAngle;
	};

	static const ImGuiResizeBorderDef resize_border_def[4] =
	{
		{ ImVec2(+1, 0), ImVec2(0, 1), ImVec2(0, 0), IM_PI * 1.00f }, // Left
		{ ImVec2(-1, 0), ImVec2(1, 0), ImVec2(1, 1), IM_PI * 0.00f }, // Right
		{ ImVec2(0, +1), ImVec2(0, 0), ImVec2(1, 0), IM_PI * 1.50f }, // Up
		{ ImVec2(0, -1), ImVec2(1, 1), ImVec2(0, 1), IM_PI * 0.50f }  // Down
	};

	auto GetResizeBorderRect = [](ImGuiWindow* window, int border_n, float perp_padding, float thickness)
	{
		ImRect rect = window->Rect();
		if (thickness == 0.0f)
		{
			rect.Max.x -= 1;
			rect.Max.y -= 1;
		}
		if (border_n == ImGuiDir_Left) { return ImRect(rect.Min.x - thickness, rect.Min.y + perp_padding, rect.Min.x + thickness, rect.Max.y - perp_padding); }
		if (border_n == ImGuiDir_Right) { return ImRect(rect.Max.x - thickness, rect.Min.y + perp_padding, rect.Max.x + thickness, rect.Max.y - perp_padding); }
		if (border_n == ImGuiDir_Up) { return ImRect(rect.Min.x + perp_padding, rect.Min.y - thickness, rect.Max.x - perp_padding, rect.Min.y + thickness); }
		if (border_n == ImGuiDir_Down) { return ImRect(rect.Min.x + perp_padding, rect.Max.y - thickness, rect.Max.x - perp_padding, rect.Max.y + thickness); }
		IM_ASSERT(0);
		return ImRect();
	};


	ImGuiContext& g = *GImGui;
	float rounding = window->WindowRounding;
	float border_size = 1.0f; // window->WindowBorderSize;
	if (border_size > 0.0f && !(window->Flags & ImGuiWindowFlags_NoBackground))
		window->DrawList->AddRect(window->Pos, { window->Pos.x + window->Size.x,  window->Pos.y + window->Size.y }, ImGui::GetColorU32(ImGuiCol_Border), rounding, 0, border_size);

	int border_held = window->ResizeBorderHeld;
	if (border_held != -1)
	{
		const ImGuiResizeBorderDef& def = resize_border_def[border_held];
		ImRect border_r = GetResizeBorderRect(window, border_held, rounding, 0.0f);
		ImVec2 p1 = ImLerp(border_r.Min, border_r.Max, def.SegmentN1);
		const float offsetX = def.InnerDir.x * rounding;
		const float offsetY = def.InnerDir.y * rounding;
		p1.x += 0.5f + offsetX;
		p1.y += 0.5f + offsetY;

		ImVec2 p2 = ImLerp(border_r.Min, border_r.Max, def.SegmentN2);
		p2.x += 0.5f + offsetX;
		p2.y += 0.5f + offsetY;

		window->DrawList->PathArcTo(p1, rounding, def.OuterAngle - IM_PI * 0.25f, def.OuterAngle);
		window->DrawList->PathArcTo(p2, rounding, def.OuterAngle, def.OuterAngle + IM_PI * 0.25f);
		window->DrawList->PathStroke(ImGui::GetColorU32(ImGuiCol_SeparatorActive), 0, ImMax(2.0f, border_size)); // Thicker than usual
	}
	if (g.Style.FrameBorderSize > 0 && !(window->Flags & ImGuiWindowFlags_NoTitleBar) && !window->DockIsActive)
	{
		float y = window->Pos.y + window->TitleBarHeight() - 1;
		window->DrawList->AddLine(ImVec2(window->Pos.x + border_size, y), ImVec2(window->Pos.x + window->Size.x - border_size, y), ImGui::GetColorU32(ImGuiCol_Border), g.Style.FrameBorderSize);
	}
}

void Sandbox::HandleManualWindowResize()
{
	auto* window = static_cast<GLFWwindow*>(Volt::Application::Get().GetWindow().GetNativeWindow());
	const bool maximized = (bool)glfwGetWindowAttrib(window, GLFW_MAXIMIZED);

	ImVec2 newSize, newPosition;
	if (!maximized && UpdateWindowManualResize(ImGui::GetCurrentWindow(), newSize, newPosition))
	{
	}
}

bool Sandbox::UpdateWindowManualResize(ImGuiWindow* window, ImVec2& newSize, ImVec2& newPosition)
{
	auto CalcWindowSizeAfterConstraint = [](ImGuiWindow* window, const ImVec2& size_desired)
	{
		ImGuiContext& g = *GImGui;
		ImVec2 new_size = size_desired;
		if (g.NextWindowData.Flags & ImGuiNextWindowDataFlags_HasSizeConstraint)
		{
			// Using -1,-1 on either X/Y axis to preserve the current size.
			ImRect cr = g.NextWindowData.SizeConstraintRect;
			new_size.x = (cr.Min.x >= 0 && cr.Max.x >= 0) ? ImClamp(new_size.x, cr.Min.x, cr.Max.x) : window->SizeFull.x;
			new_size.y = (cr.Min.y >= 0 && cr.Max.y >= 0) ? ImClamp(new_size.y, cr.Min.y, cr.Max.y) : window->SizeFull.y;
			if (g.NextWindowData.SizeCallback)
			{
				ImGuiSizeCallbackData data;
				data.UserData = g.NextWindowData.SizeCallbackUserData;
				data.Pos = window->Pos;
				data.CurrentSize = window->SizeFull;
				data.DesiredSize = new_size;
				g.NextWindowData.SizeCallback(&data);
				new_size = data.DesiredSize;
			}
			new_size.x = IM_FLOOR(new_size.x);
			new_size.y = IM_FLOOR(new_size.y);
		}

		// Minimum size
		if (!(window->Flags & (ImGuiWindowFlags_ChildWindow | ImGuiWindowFlags_AlwaysAutoResize)))
		{
			ImGuiWindow* window_for_height = (window->DockNodeAsHost && window->DockNodeAsHost->VisibleWindow) ? window->DockNodeAsHost->VisibleWindow : window;
			const float decoration_up_height = window_for_height->TitleBarHeight() + window_for_height->MenuBarHeight();
			new_size = ImMax(new_size, g.Style.WindowMinSize);
			new_size.y = ImMax(new_size.y, decoration_up_height + ImMax(0.0f, g.Style.WindowRounding - 1.0f)); // Reduce artifacts with very small windows
		}
		return new_size;
	};

	auto CalcWindowAutoFitSize = [CalcWindowSizeAfterConstraint](ImGuiWindow* window, const ImVec2& size_contents)
	{
		ImGuiContext& g = *GImGui;
		ImGuiStyle& style = g.Style;
		const float decoration_up_height = window->TitleBarHeight() + window->MenuBarHeight();
		ImVec2 size_pad{ window->WindowPadding.x * 2.0f, window->WindowPadding.y * 2.0f };
		ImVec2 size_desired = { size_contents.x + size_pad.x + 0.0f, size_contents.y + size_pad.y + decoration_up_height };
		if (window->Flags & ImGuiWindowFlags_Tooltip)
		{
			// Tooltip always resize
			return size_desired;
		}
		else
		{
			// Maximum window size is determined by the viewport size or monitor size
			const bool is_popup = (window->Flags & ImGuiWindowFlags_Popup) != 0;
			const bool is_menu = (window->Flags & ImGuiWindowFlags_ChildMenu) != 0;
			ImVec2 size_min = style.WindowMinSize;
			if (is_popup || is_menu) // Popups and menus bypass style.WindowMinSize by default, but we give then a non-zero minimum size to facilitate understanding problematic cases (e.g. empty popups)
				size_min = ImMin(size_min, ImVec2(4.0f, 4.0f));

			// FIXME-VIEWPORT-WORKAREA: May want to use GetWorkSize() instead of Size depending on the type of windows?
			ImVec2 avail_size = window->Viewport->Size;
			if (window->ViewportOwned)
				avail_size = ImVec2(FLT_MAX, FLT_MAX);
			const int monitor_idx = window->ViewportAllowPlatformMonitorExtend;
			if (monitor_idx >= 0 && monitor_idx < g.PlatformIO.Monitors.Size)
				avail_size = g.PlatformIO.Monitors[monitor_idx].WorkSize;
			ImVec2 size_auto_fit = ImClamp(size_desired, size_min, ImMax(size_min, { avail_size.x - style.DisplaySafeAreaPadding.x * 2.0f,
																					avail_size.y - style.DisplaySafeAreaPadding.y * 2.0f }));

			// When the window cannot fit all contents (either because of constraints, either because screen is too small),
			// we are growing the size on the other axis to compensate for expected scrollbar. FIXME: Might turn bigger than ViewportSize-WindowPadding.
			ImVec2 size_auto_fit_after_constraint = CalcWindowSizeAfterConstraint(window, size_auto_fit);
			bool will_have_scrollbar_x = (size_auto_fit_after_constraint.x - size_pad.x - 0.0f < size_contents.x && !(window->Flags & ImGuiWindowFlags_NoScrollbar) && (window->Flags & ImGuiWindowFlags_HorizontalScrollbar)) || (window->Flags & ImGuiWindowFlags_AlwaysHorizontalScrollbar);
			bool will_have_scrollbar_y = (size_auto_fit_after_constraint.y - size_pad.y - decoration_up_height < size_contents.y && !(window->Flags& ImGuiWindowFlags_NoScrollbar)) || (window->Flags & ImGuiWindowFlags_AlwaysVerticalScrollbar);
			if (will_have_scrollbar_x)
				size_auto_fit.y += style.ScrollbarSize;
			if (will_have_scrollbar_y)
				size_auto_fit.x += style.ScrollbarSize;
			return size_auto_fit;
		}
	};

	ImGuiContext& g = *GImGui;

	// Decide if we are going to handle borders and resize grips
	const bool handle_borders_and_resize_grips = (window->DockNodeAsHost || !window->DockIsActive);

	if (!handle_borders_and_resize_grips || window->Collapsed)
		return false;

	const ImVec2 size_auto_fit = CalcWindowAutoFitSize(window, window->ContentSizeIdeal);

	// Handle manual resize: Resize Grips, Borders, Gamepad
	int border_held = -1;
	ImU32 resize_grip_col[4] = {};
	const int resize_grip_count = g.IO.ConfigWindowsResizeFromEdges ? 2 : 1; // Allow resize from lower-left if we have the mouse cursor feedback for it.
	const float resize_grip_draw_size = IM_FLOOR(ImMax(g.FontSize * 1.10f, window->WindowRounding + 1.0f + g.FontSize * 0.2f));
	window->ResizeBorderHeld = (signed char)border_held;

	//const ImRect& visibility_rect;

	struct ImGuiResizeBorderDef
	{
		ImVec2 InnerDir;
		ImVec2 SegmentN1, SegmentN2;
		float  OuterAngle;
	};
	static const ImGuiResizeBorderDef resize_border_def[4] =
	{
		{ ImVec2(+1, 0), ImVec2(0, 1), ImVec2(0, 0), IM_PI * 1.00f }, // Left
		{ ImVec2(-1, 0), ImVec2(1, 0), ImVec2(1, 1), IM_PI * 0.00f }, // Right
		{ ImVec2(0, +1), ImVec2(0, 0), ImVec2(1, 0), IM_PI * 1.50f }, // Up
		{ ImVec2(0, -1), ImVec2(1, 1), ImVec2(0, 1), IM_PI * 0.50f }  // Down
	};

	// Data for resizing from corner
	struct ImGuiResizeGripDef
	{
		ImVec2  CornerPosN;
		ImVec2  InnerDir;
		int     AngleMin12, AngleMax12;
	};
	static const ImGuiResizeGripDef resize_grip_def[4] =
	{
		{ ImVec2(1, 1), ImVec2(-1, -1), 0, 3 },  // Lower-right
		{ ImVec2(0, 1), ImVec2(+1, -1), 3, 6 },  // Lower-left
		{ ImVec2(0, 0), ImVec2(+1, +1), 6, 9 },  // Upper-left (Unused)
		{ ImVec2(1, 0), ImVec2(-1, +1), 9, 12 }  // Upper-right (Unused)
	};

	auto CalcResizePosSizeFromAnyCorner = [CalcWindowSizeAfterConstraint](ImGuiWindow* window, const ImVec2& corner_target, const ImVec2& corner_norm, ImVec2* out_pos, ImVec2* out_size)
	{
		ImVec2 pos_min = ImLerp(corner_target, window->Pos, corner_norm);                // Expected window upper-left
		ImVec2 pos_max = ImLerp({ window->Pos.x + window->Size.x, window->Pos.y + window->Size.y }, corner_target, corner_norm); // Expected window lower-right
		ImVec2 size_expected = { pos_max.x - pos_min.x,  pos_max.y - pos_min.y };
		ImVec2 size_constrained = CalcWindowSizeAfterConstraint(window, size_expected);
		*out_pos = pos_min;
		if (corner_norm.x == 0.0f)
			out_pos->x -= (size_constrained.x - size_expected.x);
		if (corner_norm.y == 0.0f)
			out_pos->y -= (size_constrained.y - size_expected.y);
		*out_size = size_constrained;
	};

	auto GetResizeBorderRect = [](ImGuiWindow* window, int border_n, float perp_padding, float thickness)
	{
		ImRect rect = window->Rect();
		if (thickness == 0.0f)
		{
			rect.Max.x -= 1;
			rect.Max.y -= 1;
		}
		if (border_n == ImGuiDir_Left) { return ImRect(rect.Min.x - thickness, rect.Min.y + perp_padding, rect.Min.x + thickness, rect.Max.y - perp_padding); }
		if (border_n == ImGuiDir_Right) { return ImRect(rect.Max.x - thickness, rect.Min.y + perp_padding, rect.Max.x + thickness, rect.Max.y - perp_padding); }
		if (border_n == ImGuiDir_Up) { return ImRect(rect.Min.x + perp_padding, rect.Min.y - thickness, rect.Max.x - perp_padding, rect.Min.y + thickness); }
		if (border_n == ImGuiDir_Down) { return ImRect(rect.Min.x + perp_padding, rect.Max.y - thickness, rect.Max.x - perp_padding, rect.Max.y + thickness); }
		IM_ASSERT(0);
		return ImRect();
	};

	static const float WINDOWS_HOVER_PADDING = 4.0f;                        // Extend outside window for hovering/resizing (maxxed with TouchPadding) and inside windows for borders. Affect FindHoveredWindow().
	static const float WINDOWS_RESIZE_FROM_EDGES_FEEDBACK_TIMER = 0.04f;    // Reduce visual noise by only highlighting the border after a certain time.

	auto& style = g.Style;
	ImGuiWindowFlags flags = window->Flags;

	if (/*(flags & ImGuiWindowFlags_NoResize) || */(flags & ImGuiWindowFlags_AlwaysAutoResize) || window->AutoFitFramesX > 0 || window->AutoFitFramesY > 0)
		return false;
	if (window->WasActive == false) // Early out to avoid running this code for e.g. an hidden implicit/fallback Debug window.
		return false;

	bool ret_auto_fit = false;
	const int resize_border_count = g.IO.ConfigWindowsResizeFromEdges ? 4 : 0;
	const float grip_draw_size = IM_FLOOR(ImMax(g.FontSize * 1.35f, window->WindowRounding + 1.0f + g.FontSize * 0.2f));
	const float grip_hover_inner_size = IM_FLOOR(grip_draw_size * 0.75f);
	const float grip_hover_outer_size = g.IO.ConfigWindowsResizeFromEdges ? WINDOWS_HOVER_PADDING : 0.0f;

	ImVec2 pos_target(FLT_MAX, FLT_MAX);
	ImVec2 size_target(FLT_MAX, FLT_MAX);

	// Calculate the range of allowed position for that window (to be movable and visible past safe area padding)
	// When clamping to stay visible, we will enforce that window->Pos stays inside of visibility_rect.
	ImRect viewport_rect(window->Viewport->GetMainRect());
	ImRect viewport_work_rect(window->Viewport->GetWorkRect());
	ImVec2 visibility_padding = ImMax(style.DisplayWindowPadding, style.DisplaySafeAreaPadding);
	ImRect visibility_rect({ viewport_work_rect.Min.x + visibility_padding.x, viewport_work_rect.Min.y + visibility_padding.y },
		{ viewport_work_rect.Max.x - visibility_padding.x, viewport_work_rect.Max.y - visibility_padding.y });

	// Clip mouse interaction rectangles within the viewport rectangle (in practice the narrowing is going to happen most of the time).
	// - Not narrowing would mostly benefit the situation where OS windows _without_ decoration have a threshold for hovering when outside their limits.
	//   This is however not the case with current backends under Win32, but a custom borderless window implementation would benefit from it.
	// - When decoration are enabled we typically benefit from that distance, but then our resize elements would be conflicting with OS resize elements, so we also narrow.
	// - Note that we are unable to tell if the platform setup allows hovering with a distance threshold (on Win32, decorated window have such threshold).
	// We only clip interaction so we overwrite window->ClipRect, cannot call PushClipRect() yet as DrawList is not yet setup.
	const bool clip_with_viewport_rect = !(g.IO.BackendFlags & ImGuiBackendFlags_HasMouseHoveredViewport) || (g.IO.MouseHoveredViewport != window->ViewportId) || !(window->Viewport->Flags & ImGuiViewportFlags_NoDecoration);
	if (clip_with_viewport_rect)
		window->ClipRect = window->Viewport->GetMainRect();

	// Resize grips and borders are on layer 1
	window->DC.NavLayerCurrent = ImGuiNavLayer_Menu;

	// Manual resize grips
	ImGui::PushID("#RESIZE");
	for (int resize_grip_n = 0; resize_grip_n < resize_grip_count; resize_grip_n++)
	{
		const ImGuiResizeGripDef& def = resize_grip_def[resize_grip_n];

		const ImVec2 corner = ImLerp(window->Pos, { window->Pos.x + window->Size.x, window->Pos.y + window->Size.y }, def.CornerPosN);

		// Using the FlattenChilds button flag we make the resize button accessible even if we are hovering over a child window
		bool hovered, held;
		const ImVec2 min = { corner.x - def.InnerDir.x * grip_hover_outer_size, corner.y - def.InnerDir.y * grip_hover_outer_size };
		const ImVec2 max = { corner.x + def.InnerDir.x * grip_hover_outer_size, corner.y + def.InnerDir.y * grip_hover_outer_size };
		ImRect resize_rect(min, max);

		if (resize_rect.Min.x > resize_rect.Max.x) ImSwap(resize_rect.Min.x, resize_rect.Max.x);
		if (resize_rect.Min.y > resize_rect.Max.y) ImSwap(resize_rect.Min.y, resize_rect.Max.y);
		ImGuiID resize_grip_id = window->GetID(resize_grip_n); // == GetWindowResizeCornerID()
		ImGui::ButtonBehavior(resize_rect, resize_grip_id, &hovered, &held, ImGuiButtonFlags_FlattenChildren | ImGuiButtonFlags_NoNavFocus);
		//GetForegroundDrawList(window)->AddRect(resize_rect.Min, resize_rect.Max, IM_COL32(255, 255, 0, 255));
		if (hovered || held)
			g.MouseCursor = (resize_grip_n & 1) ? ImGuiMouseCursor_ResizeNESW : ImGuiMouseCursor_ResizeNWSE;

		if (held && g.IO.MouseDoubleClicked[0] && resize_grip_n == 0)
		{
			// Manual auto-fit when double-clicking
			size_target = CalcWindowSizeAfterConstraint(window, size_auto_fit);
			ret_auto_fit = true;
			ImGui::ClearActiveID();
		}
		else if (held)
		{
			// Resize from any of the four corners
			// We don't use an incremental MouseDelta but rather compute an absolute target size based on mouse position
			ImVec2 clamp_min = ImVec2(def.CornerPosN.x == 1.0f ? visibility_rect.Min.x : -FLT_MAX, def.CornerPosN.y == 1.0f ? visibility_rect.Min.y : -FLT_MAX);
			ImVec2 clamp_max = ImVec2(def.CornerPosN.x == 0.0f ? visibility_rect.Max.x : +FLT_MAX, def.CornerPosN.y == 0.0f ? visibility_rect.Max.y : +FLT_MAX);

			const float x = g.IO.MousePos.x - g.ActiveIdClickOffset.x + ImLerp(def.InnerDir.x * grip_hover_outer_size, def.InnerDir.x * -grip_hover_inner_size, def.CornerPosN.x);
			const float y = g.IO.MousePos.y - g.ActiveIdClickOffset.y + ImLerp(def.InnerDir.y * grip_hover_outer_size, def.InnerDir.y * -grip_hover_inner_size, def.CornerPosN.y);

			ImVec2 corner_target(x, y); // Corner of the window corresponding to our corner grip
			corner_target = ImClamp(corner_target, clamp_min, clamp_max);
			CalcResizePosSizeFromAnyCorner(window, corner_target, def.CornerPosN, &pos_target, &size_target);
		}

		// Only lower-left grip is visible before hovering/activating
		if (resize_grip_n == 0 || held || hovered)
			resize_grip_col[resize_grip_n] = ImGui::GetColorU32(held ? ImGuiCol_ResizeGripActive : hovered ? ImGuiCol_ResizeGripHovered : ImGuiCol_ResizeGrip);
	}
	for (int border_n = 0; border_n < resize_border_count; border_n++)
	{
		const ImGuiResizeBorderDef& def = resize_border_def[border_n];
		const ImGuiAxis axis = (border_n == ImGuiDir_Left || border_n == ImGuiDir_Right) ? ImGuiAxis_X : ImGuiAxis_Y;

		bool hovered, held;
		ImRect border_rect = GetResizeBorderRect(window, border_n, grip_hover_inner_size, WINDOWS_HOVER_PADDING);
		ImGuiID border_id = window->GetID(border_n + 4); // == GetWindowResizeBorderID()
		ImGui::ButtonBehavior(border_rect, border_id, &hovered, &held, ImGuiButtonFlags_FlattenChildren);
		//GetForegroundDrawLists(window)->AddRect(border_rect.Min, border_rect.Max, IM_COL32(255, 255, 0, 255));
		if ((hovered && g.HoveredIdTimer > WINDOWS_RESIZE_FROM_EDGES_FEEDBACK_TIMER) || held)
		{
			g.MouseCursor = (axis == ImGuiAxis_X) ? ImGuiMouseCursor_ResizeEW : ImGuiMouseCursor_ResizeNS;
			if (held)
				border_held = border_n;
		}
		if (held)
		{
			ImVec2 clamp_min(border_n == ImGuiDir_Right ? visibility_rect.Min.x : -FLT_MAX, border_n == ImGuiDir_Down ? visibility_rect.Min.y : -FLT_MAX);
			ImVec2 clamp_max(border_n == ImGuiDir_Left ? visibility_rect.Max.x : +FLT_MAX, border_n == ImGuiDir_Up ? visibility_rect.Max.y : +FLT_MAX);
			ImVec2 border_target = window->Pos;
			border_target[axis] = g.IO.MousePos[axis] - g.ActiveIdClickOffset[axis] + WINDOWS_HOVER_PADDING;
			border_target = ImClamp(border_target, clamp_min, clamp_max);
			CalcResizePosSizeFromAnyCorner(window, border_target, ImMin(def.SegmentN1, def.SegmentN2), &pos_target, &size_target);
		}
	}
	ImGui::PopID();

	bool changed = false;
	newSize = window->Size;
	newPosition = window->Pos;

	// Apply back modified position/size to window
	if (size_target.x != FLT_MAX)
	{
		//window->SizeFull = size_target;
		//MarkIniSettingsDirty(window);
		newSize = size_target;
		changed = true;
	}
	if (pos_target.x != FLT_MAX)
	{
		//window->Pos = ImFloor(pos_target);
		//MarkIniSettingsDirty(window);
		newPosition = pos_target;
		changed = true;
	}

	//window->Size = window->SizeFull;
	return changed;
}

float Sandbox::DrawTitlebar()
{
	const float titlebarHeight = 50.f;
	const ImVec2 windowPadding = ImGui::GetCurrentWindow()->WindowPadding;

	ImGui::SetCursorPos(ImVec2(windowPadding.x, windowPadding.y));
	const ImVec2 titlebarMin = ImGui::GetCursorScreenPos();
	const ImVec2 titlebarMax = { ImGui::GetCursorScreenPos().x + ImGui::GetWindowWidth() - windowPadding.y * 2.0f,
								 ImGui::GetCursorScreenPos().y + titlebarHeight };
	auto* drawList = ImGui::GetWindowDrawList();
	drawList->AddRectFilled(titlebarMin, titlebarMax, ImColor(0.2f, 0.2f, 0.2f, 1.000f));

	const float buttonSize = 13.f;
	const float iconSize = 40.f;

	const uint32_t buttonCount = 4;
	const float buttonPadding = ImGui::GetStyle().FramePadding.x;

	const float buttonsAreaWidth = buttonSize * buttonCount + buttonPadding * buttonCount + 20.f;

	//UI::ShiftCursor(4.f, 4.f);

	ImGui::Image(UI::GetTextureID(EditorIconLibrary::GetIcon(EditorIcon::Volt)), { iconSize, iconSize });
	ImGui::SameLine();

	// Menu bar
	{
		DrawMenuBar();
	}

	ImGui::SameLine();

	const float w = ImGui::GetContentRegionAvail().x;
	ImGui::InvisibleButton("##titlebarDragZone", ImVec2(w - buttonsAreaWidth, titlebarHeight));
	myTitlebarHovered = ImGui::IsItemHovered();

	ImGui::SameLine();

	UI::ShiftCursor(ImGui::GetContentRegionAvail().x - buttonsAreaWidth, 4.f);

	// Close, minimize/maximize 
	{
		UI::ScopedColor button(ImGuiCol_Button, { 0.f, 0.f, 0.f, 0.f });

		if (UI::ImageButton("##minimize", UI::GetTextureID(EditorIconLibrary::GetIcon(EditorIcon::Minimize)), { buttonSize, buttonSize }))
		{
			Volt::Application::Get().GetWindow().Minimize();
		}

		ImGui::SameLine();

		const bool isMaximized = Volt::Application::Get().GetWindow().IsMaximized();
		Ref<Volt::Texture2D> maximizeTexture = isMaximized ? EditorIconLibrary::GetIcon(EditorIcon::Windowize) : EditorIconLibrary::GetIcon(EditorIcon::Maximize);

		if (UI::ImageButton("##maximize", UI::GetTextureID(maximizeTexture), { buttonSize, buttonSize }))
		{
			if (!isMaximized)
			{
				Volt::Application::Get().GetWindow().Maximize();
			}
			else
			{
				Volt::Application::Get().GetWindow().Restore();
			}
		}

		ImGui::SameLine();

		if (UI::ImageButton("##close", UI::GetTextureID(EditorIconLibrary::GetIcon(EditorIcon::Close)), { buttonSize, buttonSize }))
		{
			Volt::WindowCloseEvent e{};
			Volt::Application::Get().OnEvent(e);
		}
	}

	return titlebarHeight;
}

void Sandbox::DrawMenuBar()
{
	const ImRect menuBarRect = { ImGui::GetCursorPos(), {ImGui::GetContentRegionAvail().x, ImGui::GetFrameHeightWithSpacing()} };

	ImGui::BeginGroup();
	if (UI::BeginMenuBar(menuBarRect))
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("New Scene", "Ctrl + N"))
			{
				UI::OpenModal("Do you want to save scene?##NewScene");
			}

			if (ImGui::MenuItem("Open...", "Ctrl + O"))
			{
				UI::OpenModal("Do you want to save scene?##OpenScene");
			}

			if (ImGui::MenuItem("Save As", "Ctrl + Shift + S"))
			{
				SaveSceneAs();
			}

			if (ImGui::MenuItem("Save", "Ctrl + S"))
			{
				SaveScene();
			}

			if (ImGui::MenuItem("Build Game"))
			{
				UI::OpenModal("Build");
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Edit"))
		{
			if (ImGui::MenuItem("Undo", "Ctrl + Z"))
			{
				ExecuteUndo();
			}

			if (ImGui::MenuItem("Redo", "Ctrl + Y"))
			{
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Tools"))
		{
			if (ImGui::BeginMenu("Maya"))
			{
				if (ImGui::MenuItem("Install Maya tools..."))
				{
					InstallMayaTools();
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Asset Browser"))
			{
				if (ImGui::MenuItem("Open new Asset Browser"))
				{
					myEditorWindows.emplace_back(CreateRef<AssetBrowserPanel>(myRuntimeScene, "##Secondary" + std::to_string(myAssetBrowserCount++)));
				}

				ImGui::EndMenu();
			}

			for (const auto& window : myEditorWindows)
			{
				ImGui::MenuItem(window->GetTitle().c_str(), "", &const_cast<bool&>(window->IsOpen()));
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Editor"))
		{
			if (ImGui::MenuItem("Reset layout"))
			{
				myShouldResetLayout = true;
			}

			if (ImGui::MenuItem("Crash"))
			{
				int* ptr = nullptr;
				*ptr = 0;
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Engine"))
		{
			if (ImGui::MenuItem("Recompile all shaders"))
			{
				for (const auto& [name, shader] : Volt::ShaderRegistry::GetAllShaders())
				{
					shader->Reload(true);
				}
			}

			if (ImGui::MenuItem("Reload C#"))
			{
				Volt::MonoScriptEngine::ReloadAssembly();
			}

			ImGui::EndMenu();
		}

		UI::EndMenuBar();
	}
	ImGui::EndGroup();
}

void Sandbox::SaveSceneAsModal()
{
	UI::ScopedStyleFloat buttonRounding{ ImGuiStyleVar_FrameRounding, 2.f };

	if (UI::BeginModal("Save As"))
	{
		UI::PushId();
		if (UI::BeginProperties("saveSceneAs"))
		{
			UI::Property("Name", mySaveSceneData.name);
			UI::Property("Destination", mySaveSceneData.destinationPath);

			UI::EndProperties();
		}
		UI::PopId();

		ImGui::PushItemWidth(80.f);
		if (ImGui::Button("Save"))
		{
			if (mySaveSceneData.name.empty())
			{
				ImGui::CloseCurrentPopup();

				UI::Notify(NotificationType::Error, "Unable to save scene!", "A scene with no name cannot be saved!");

				ImGui::PopItemWidth();
				UI::EndModal();
				return;
			}

			const std::filesystem::path destPath = mySaveSceneData.destinationPath / mySaveSceneData.name;
			if (!FileSystem::Exists(destPath))
			{
				std::filesystem::create_directories(destPath);
			}

			myRuntimeScene->path = destPath;
			myRuntimeScene->handle = Volt::AssetHandle{};
			Volt::AssetManager::Get().SaveAsset(myRuntimeScene);

			UI::Notify(NotificationType::Success, "Successfully saved scene!", std::format("Scene {0} was saved successfully!", mySaveSceneData.name));
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel"))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::PopItemWidth();

		UI::EndModal();
	}
}

void Sandbox::BuildGameModal()
{
	UI::ScopedStyleFloat buttonRounding{ ImGuiStyleVar_FrameRounding, 2.f };

	if (UI::BeginModal("Build", ImGuiWindowFlags_AlwaysAutoResize))
	{
		UI::PushId();
		if (UI::BeginProperties("buildData"))
		{
			UI::PropertyDirectory("Build Path", myBuildInfo.buildDirectory);

			ImGui::Separator();

			for (auto& handle : myBuildInfo.sceneHandles)
			{
				EditorUtils::Property("Scene", handle, Volt::AssetType::Scene);
			}

			UI::EndProperties();
		}
		UI::PopId();

		if (ImGui::Button("Add Scene"))
		{
			myBuildInfo.sceneHandles.emplace_back();
		}

		ImGui::PushItemWidth(80.f);
		if (ImGui::Button("Build"))
		{
			GameBuilder::BuildGame(myBuildInfo);
			myBuildStarted = true;
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel"))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::PopItemWidth();

		UI::EndModal();
	}
}

void Sandbox::RenderProgressBar(float progress)
{
	const auto vp_size = ImGui::GetMainViewport()->Size;

	constexpr auto windowFlags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoFocusOnAppearing;

	ImGui::SetNextWindowBgAlpha(1.f);
	ImGui::SetNextWindowPos(ImVec2(vp_size.x - NOTIFY_PADDING_X, 100.f), ImGuiCond_Always, ImVec2(1.0f, 1.0f));
	ImGui::Begin("##progressBar", nullptr, windowFlags);

	{
		ImGui::TextUnformatted("Build Progress");
		ImGui::ProgressBar(progress);

		ImGui::Text("Current File: %s", GameBuilder::GetCurrentFile().c_str());
	}

	ImGui::End();
}