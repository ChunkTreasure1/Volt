#pragma once
#include <Volt/Scene/Scene.h>
#include "Volt/Core/Base.h"
#include "Sandbox/Window/EditorWindow.h"
#include <Volt/Scene/Scene.h>
#include "GEM/gem.h"
#include "Game/Tools/Spline.h"

class SplinePanel : public EditorWindow
{
public:
	SplinePanel(Ref<Volt::Scene>& aScene);
	void UpdateMainContent() override;

private:
	Ref<Volt::Scene>& myCurrentScene;
	Ref<Spline> mySpline;

	bool mySplineCreated = false;
};