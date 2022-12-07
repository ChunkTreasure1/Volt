#pragma once

#include <Volt/Components/VTCinemaComponents.h>
#include <Volt/Components/Components.h>

#include <Volt/Scene/Scene.h>
#include <Volt/Core/Base.h>
#include <Volt/Events/ApplicationEvent.h>

#include "Sandbox/Window/EditorWindow.h"

class VTCinemaPanel : public EditorWindow
{
public:
	VTCinemaPanel(Ref<Volt::Scene>& aScene);

	void UpdateMainContent() override;
	void UpdateContent() override;
	void OnEvent(Volt::Event& e) override;

	void UpdateCameraProperties();
	void UpdateSetDetails();

	bool RelocateSetOnLoad(Volt::OnSceneLoadedEvent& e);
	bool SaveChangedProperties(Volt::OnSceneStopEvent& e);

	void InitCinema();
	void GetCinema();
	
	Volt::Entity GetSelectedCamera() { return myVTCams[mySelectedEntity]; }

	Volt::Entity CreateDirector();
	Volt::Entity GetDirector();

	Volt::Entity CreateNewCamera();
	const std::vector<Volt::Entity> GetAllCameras();
	void UpdateSelectedCamera();

private:
	Volt::Entity myCurrentDirector = Volt::Entity{ 0,nullptr };
	int mySelectedEntity = -1;

	std::vector<std::pair<Volt::CameraComponent, Volt::VTCamComponent>> myVTCamComponents;
	std::vector<Volt::Entity> myVTCams;

	bool myNewSettingsSaved = false;
	bool myToManyDirectors = false;

	Ref<Volt::Scene>& myCurrentScene;
};