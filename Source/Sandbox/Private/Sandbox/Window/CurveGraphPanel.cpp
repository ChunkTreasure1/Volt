#include "sbpch.h"
#include "CurveGraphPanel.h"

#include <imgui_bezier.h>

CurveGraphPanel::CurveGraphPanel() : EditorWindow("Curve Graph Test")
{
}

void CurveGraphPanel::UpdateMainContent()
{
	static float x = 0.5f;
	static float v[5] = { 1.f, 0.f, 1.f, 1.f };

	ImGui::Bezier("test", v);
	float y = ImGui::BezierValue(x, v);

	ImGui::SliderFloat("x", &x, 0.f, 1.f);
	ImGui::InputFloat("y", &y);
}
