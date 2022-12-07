#include "sbpch.h"

#include "VTCinema.h"

#include <Volt/Scripting/ScriptRegistry.h>
#include <Volt/Log/Log.h>
#include <Volt/Utility/UIUtility.h>

#include <Volt/VTCinema/Director.h>

VTCinemaPanel::VTCinemaPanel(Ref<Volt::Scene>& aScene)
	:EditorWindow("VOLT CINEMA", true), myCurrentScene(aScene)
{
}

void VTCinemaPanel::UpdateMainContent()
{
}

void VTCinemaPanel::UpdateContent()
{
	UpdateSetDetails();
	UpdateCameraProperties();
	UpdateSelectedCamera();
}

void VTCinemaPanel::OnEvent(Volt::Event& e)
{
	Volt::EventDispatcher dispatcher{ e };

	dispatcher.Dispatch<Volt::OnSceneLoadedEvent>(VT_BIND_EVENT_FN(VTCinemaPanel::RelocateSetOnLoad));
	dispatcher.Dispatch<Volt::OnSceneStopEvent>(VT_BIND_EVENT_FN(VTCinemaPanel::SaveChangedProperties));
}

void VTCinemaPanel::UpdateCameraProperties()
{
	ImGui::Begin("Camera Properties");

	UI::PushId();

	if (mySelectedEntity == -1 || myVTCams[mySelectedEntity].IsNull())
	{
		UI::PopId();
		ImGui::End();
		return;
	}

	Volt::Entity selectedCam = myVTCams[mySelectedEntity];
	auto& baseCamComp = selectedCam.GetComponent<Volt::CameraComponent>();
	auto& vtCamComp = selectedCam.GetComponent<Volt::VTCamComponent>();

	std::string labelName = "Volt Cam: " + selectedCam.GetComponent<Volt::TagComponent>().tag;
	ImGui::LabelText("##CamName", labelName.c_str());

	ImGui::Spacing();
	ImGui::Separator();

	if (UI::BeginProperties("Core Settings"))
	{
		auto& enumData = Wire::ComponentRegistry::EnumData();
		UI::ComboProperty("CameraType", *(int32_t*)&vtCamComp.cameraType, enumData.at("eCameraType"));

		if (UI::Property("FOV", vtCamComp.fov)) 
		{
			baseCamComp.fieldOfView = vtCamComp.fov;
		}

		UI::EndProperties();
	}

	ImGui::Separator();

	if (UI::BeginProperties("Transform")) 
	{
		UI::PropertyEntity("Target", myCurrentScene, vtCamComp.targetId);

		ImGui::SameLine();
		if (ImGui::Button("Clear Target")) 
		{
			vtCamComp.targetId = 0;
		}

		UI::PropertyEntity("LookAt", myCurrentScene, vtCamComp.lookAtId);

		ImGui::SameLine();
		if (ImGui::Button("Clear LookAt"))
		{
			vtCamComp.lookAtId = 0;
		}

		if (vtCamComp.targetId != 0) 
		{
			UI::Property("Offset", vtCamComp.offset);
		}

		UI::EndProperties();
	}

	UI::PopId();

	ImGui::End();
}

void VTCinemaPanel::UpdateSetDetails()
{
	ImGui::Begin("Studio");

	if (ImGui::Button("Create Cinema"))
	{
		InitCinema();
	}

	ImGui::SameLine();

	if (ImGui::Button("Open Camera Set"))
	{
		GetCinema();
	}

	ImGui::Spacing();

	ImGui::Separator();

	ImGui::LabelText("##Cams", "Cameras On Set");

	if (ImGui::Button("New VTCam"))
	{
		myVTCams.push_back(CreateNewCamera());
	}

	if (myVTCams.empty())
	{
		ImGui::End();
		return;
	}

	static int selected = -1;
	for (int n = 0; n < myVTCams.size(); n++)
	{
		if (!myVTCams[n].IsNull())
		{
			auto& tagComp = myVTCams[n].GetComponent<Volt::TagComponent>();

			char buf[32];

			sprintf_s(buf, tagComp.tag.c_str(), n);

			if (ImGui::Selectable(buf, selected == n))
				selected = n;
		}
		else
		{
			myVTCams.erase(myVTCams.begin() + n);
		}
	}

	if (selected >= 0)
	{
		if (selected != mySelectedEntity)
		{
			for (auto& cam : myVTCams)
			{
				cam.GetComponent<Volt::CameraComponent>().priority = 1;
			}

			myVTCams[selected].GetComponent<Volt::CameraComponent>().priority = 0;
		}

		mySelectedEntity = selected;
	}

	ImGui::End();
}

bool VTCinemaPanel::RelocateSetOnLoad(Volt::OnSceneLoadedEvent& e)
{
	myCurrentDirector = Volt::Entity{ 0,nullptr };
	myCurrentDirector = GetDirector();

	myVTCams.clear();
	myVTCams = GetAllCameras();

	if (myNewSettingsSaved && e.GetScene() == myCurrentScene) 
	{
		for (size_t i = 0; i < myVTCams.size(); i++)
		{
			myVTCams[i].GetComponent<Volt::CameraComponent>() = myVTCamComponents[i].first;
			myVTCams[i].GetComponent<Volt::VTCamComponent>() = myVTCamComponents[i].second;
		}

		myNewSettingsSaved = false;
	}

	return false;
}

bool VTCinemaPanel::SaveChangedProperties(Volt::OnSceneStopEvent& e)
{
	myVTCamComponents.clear();
	for (auto& cam : myVTCams)
	{
		myVTCamComponents.emplace_back(std::pair(cam.GetComponent<Volt::CameraComponent>(), cam.GetComponent<Volt::VTCamComponent>()));
	}
	myNewSettingsSaved = true;

	return false;
}

void VTCinemaPanel::InitCinema()
{
	if (myCurrentDirector.IsNull())
	{
		myCurrentDirector = GetDirector();

		if (myToManyDirectors == true) return;

		if (!myCurrentDirector)
		{
			myCurrentDirector = CreateDirector();
		}
	}

	if (myVTCams.empty())
	{
		myVTCams.emplace_back(CreateNewCamera());
	}
}

void VTCinemaPanel::GetCinema()
{
	if (myCurrentDirector.IsNull())
	{
		myCurrentDirector = GetDirector();

		if (myToManyDirectors == true) return;
	}

	myVTCams.clear();
	myVTCams = GetAllCameras();
}

Volt::Entity VTCinemaPanel::GetDirector()
{
	const std::vector<Wire::EntityId> directorsIDs = myCurrentScene->GetAllEntitiesWith<Volt::DirectorComponent>();

	if (directorsIDs.size() > 1)
	{
		VT_CORE_ERROR("VTCINEMA: More then 1 director loacated. Please remove one!");
		myToManyDirectors = true;

		return Volt::Entity{ 0,nullptr };
	}

	if (directorsIDs.size() == 1)
	{
		return Volt::Entity{ directorsIDs[0], myCurrentScene.get() };
	}

	return Volt::Entity{ 0,nullptr };
}

const std::vector<Volt::Entity> VTCinemaPanel::GetAllCameras()
{
	std::vector<Wire::EntityId> cameraIDs = myCurrentScene->GetAllEntitiesWith<Volt::VTCamComponent>();

	if (cameraIDs.empty())
	{
		VT_CORE_ERROR("VTCINEMA: No cameras loacated!");
		return std::vector<Volt::Entity>();
	}

	std::vector<Volt::Entity> vtCams;
	vtCams.reserve(cameraIDs.size());

	for (auto& id : cameraIDs)
	{
		vtCams.emplace_back(Volt::Entity{ id,myCurrentScene.get() });
	}

	return vtCams;
}

void VTCinemaPanel::UpdateSelectedCamera()
{
	if (mySelectedEntity == -1 || myVTCams[mySelectedEntity].IsNull()) 
	{
		return;
	}

	Volt::Entity selectedEnt = myVTCams[mySelectedEntity];
	auto& vtCamComp = selectedEnt.GetComponent<Volt::VTCamComponent>();

	if (vtCamComp.targetId != 0) 
	{
		Volt::Entity target = Volt::Entity{ vtCamComp.targetId, myCurrentScene.get() };
		selectedEnt.SetPosition(target.GetWorldPosition() + vtCamComp.offset);
	}

	if (vtCamComp.lookAtId != 0) 
	{
		Volt::Entity lookAtTarget = Volt::Entity{ vtCamComp.lookAtId, myCurrentScene.get() };

		gem::vec3 lookAtPos = lookAtTarget.GetWorldPosition();
		gem::mat test = gem::lookAtLH(selectedEnt.GetWorldPosition(), lookAtPos, { 0,1,0 });
		gem::vec3 rot = 0;
		gem::vec3 dump = 0;
		gem::decompose(test, dump, rot, dump);

		selectedEnt.SetRotation(rot * -1);
	}
}

Volt::Entity VTCinemaPanel::CreateDirector()
{
	Volt::Entity director = myCurrentScene->CreateEntity();

	director.GetComponent<Volt::TagComponent>().tag = "Director";

	director.AddComponent<Volt::DirectorComponent>();
	director.AddScript("Director");

	return director;
}

Volt::Entity VTCinemaPanel::CreateNewCamera()
{
	Volt::Entity camEnt = myCurrentScene->CreateEntity();

	auto& tagComp = camEnt.GetComponent<Volt::TagComponent>();
	tagComp.tag = "VTCam " + std::to_string(myVTCams.size());

	auto& vtCamComp = camEnt.AddComponent<Volt::VTCamComponent>();

	auto& baseCamComp = camEnt.AddComponent<Volt::CameraComponent>();
	baseCamComp.fieldOfView = vtCamComp.fov;

	return camEnt;
}
