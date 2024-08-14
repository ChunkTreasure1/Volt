#pragma once
#include "Sandbox/Window/EditorWindow.h"
#include <Volt/Scene/Scene.h>

struct KeyFrame
{
	uint32_t index = 0;
	float xPos = 10.f;
};

class Sequencer : public EditorWindow
{
public:
	Sequencer(Ref<Volt::Scene>& aScene);
	void UpdateMainContent() override;
	void UpdateContent() override;

private:

	void UpdateTimeLine();

	Ref<Volt::Scene>& myCurrentScene;
	Vector<KeyFrame> myKeyFrames;
};