#pragma once

#include "Volt/Vision/VisionComponents.h"
#include <Volt/Public/Components/RenderingComponents.h>

#include <Volt/Scene/Scene.h>
#include <Volt/Core/Base.h>
#include <Volt/Public/Events/ApplicationEvents.h>
#include <Volt/Public/Events/SceneEvents.h>
#include <Sandbox/Camera/EditorCameraController.h>

#include "Sandbox/Window/EditorWindow.h"

class VisionPanel : public EditorWindow
{
public:
	VisionPanel(Ref<Volt::Scene>& aScene, EditorCameraController* aEditorCamera);

	void UpdateMainContent() override;
	void UpdateContent() override;

	void UpdateCameraProperties();
	void UpdateSetDetails();

	bool RelocateSetOnLoad(Volt::OnSceneLoadedEvent& e);
	bool SaveChangedProperties(Volt::OnSceneStopEvent& e);

	void InitCinema();
	void GetCinema();

	Volt::Entity GetSelectedCamera() { return myVisionCams[mySelectedCamera]; }

	Volt::Entity CreateNewTrigger();
	const Vector<Volt::Entity> GetAllCameraTriggers();

	Volt::Entity CreateNewCamera();
	const Vector<Volt::Entity> GetAllCameras();
	void UpdateSelectedCamera();

private:
	int mySelectedCamera = -1;

	Vector<std::pair<Volt::CameraComponent, Volt::VisionCameraComponent>> myVTCamComponents;

	Vector<Volt::Entity> myVisionCams;
	Vector<Volt::Entity> myCameraTriggers;

	bool myNewSettingsSaved = false;
	bool myToManyDirectors = false;

	Ref<Volt::Scene>& myCurrentScene;
	EditorCameraController* myEditorCamera;
};
