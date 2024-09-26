#pragma once
#include "Sandbox/Window/EditorWindow.h"

#include <imgui.h>

class CurveGraphPanel : public EditorWindow
{
public:
	CurveGraphPanel();
	void UpdateMainContent() override;
};
