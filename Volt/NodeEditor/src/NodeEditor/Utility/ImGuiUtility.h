#pragma once

#include <imgui.h>

namespace NE
{
	namespace ImGuiUtility
	{
		static void ShiftCursor(float x, float y)
		{
			ImVec2 pos = { ImGui::GetCursorPosX() + x, ImGui::GetCursorPosY() + y };
			ImGui::SetCursorPos(pos);
		}
	}
}