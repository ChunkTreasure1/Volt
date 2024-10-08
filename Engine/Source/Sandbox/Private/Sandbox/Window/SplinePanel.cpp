#include "sbpch.h"
#include "Window/SplinePanel.h"


SplinePanel::SplinePanel(Ref<Volt::Scene>& aScene)
	:EditorWindow("Spline Tool"), myCurrentScene(aScene)
{
	mySpline = CreateRef<Spline>(myCurrentScene, 50);
}

void SplinePanel::UpdateMainContent()
{

	if (ImGui::Button("Init Spline"))
	{
		mySpline->InitSpline();
	}

	if (ImGui::Button("Add Segment"))
	{
		mySpline->AddSegment();
	}

	if (ImGui::Button("Draw Bezier"))
	{
		mySplineCreated = true;
	}

	if (mySplineCreated)
	{
		mySpline->UpdateSpline();
	}
}
