#pragma once

#include "Volt/Vision/VisionComponents.h"
#include <Volt/Components/RenderingComponents.h>

#include <Volt/Scene/Scene.h>
#include <Volt/Core/Base.h>
#include <Volt/Events/ApplicationEvent.h>
#include <Sandbox/Camera/EditorCameraController.h>

#include "Sandbox/Window/EditorWindow.h"

class VisionPanel : public EditorWindow
{
public:
	VisionPanel(Ref<Volt::Scene>& aScene, EditorCameraController* aEditorCamera);

	void UpdateMainContent() override;
	void UpdateContent() override;
	void OnEvent(Volt::Event& e) override;

	void UpdateCameraProperties();
	void UpdateSetDetails();

	bool RelocateSetOnLoad(Volt::OnSceneLoadedEvent& e);
	bool SaveChangedProperties(Volt::OnSceneStopEvent& e);

	void InitCinema();
	void GetCinema();

	Volt::Entity GetSelectedCamera() { return myVisionCams[mySelectedCamera]; }

	Volt::Entity CreateNewTrigger();
	const std::vector<Volt::Entity> GetAllCameraTriggers();

	Volt::Entity CreateNewCamera();
	const std::vector<Volt::Entity> GetAllCameras();
	void UpdateSelectedCamera();

private:
	int mySelectedCamera = -1;

	std::vector<std::pair<Volt::CameraComponent, Volt::VisionCameraComponent>> myVTCamComponents;

	std::vector<Volt::Entity> myVisionCams;
	std::vector<Volt::Entity> myCameraTriggers;

	bool myNewSettingsSaved = false;
	bool myToManyDirectors = false;

	Ref<Volt::Scene>& myCurrentScene;
	EditorCameraController* myEditorCamera;
};
