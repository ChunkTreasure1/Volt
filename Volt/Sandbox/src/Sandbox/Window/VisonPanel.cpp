#include "sbpch.h"

#include "VisonPanel.h"

#include <Volt/Utility/UIUtility.h>
#include <Volt/Utility/Random.h>
#include <Volt/Core/Application.h>
#include <Volt/Components/PhysicsComponents.h>
#include <Volt/Vision/VisionTrigger.h>
#include <Volt/Vision/Vision.h>

#include <Volt/Rendering/Camera/Camera.h>

VisionPanel::VisionPanel(Ref<Volt::Scene>& aScene, EditorCameraController* aEditorCamera)
	:EditorWindow("Vision", true), myCurrentScene(aScene), myEditorCamera(aEditorCamera)
{
	m_windowFlags = ImGuiWindowFlags_MenuBar;
}

void VisionPanel::UpdateMainContent()
{
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("Scene"))
		{
			if (ImGui::MenuItem("Init Base Cinema"))
			{
				InitCinema();
			}

			if (ImGui::MenuItem("Locate Set"))
			{
				GetCinema();
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("New"))
		{
			if (ImGui::MenuItem("Camera"))
			{
				myVisionCams.push_back(CreateNewCamera());
			}

			if (ImGui::MenuItem("Camera Trigger"))
			{
				myCameraTriggers.push_back(CreateNewTrigger());
			}

			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}
}

void VisionPanel::UpdateContent()
{
	UpdateSetDetails();
	UpdateCameraProperties();

	if (!Volt::Application::Get().IsRuntime())
	{
		//UpdateSelectedCamera();
	}
}

void VisionPanel::OnEvent(Volt::Event& e)
{
	Volt::EventDispatcher dispatcher{ e };

	dispatcher.Dispatch<Volt::OnSceneLoadedEvent>(VT_BIND_EVENT_FN(VisionPanel::RelocateSetOnLoad));
	dispatcher.Dispatch<Volt::OnSceneStopEvent>(VT_BIND_EVENT_FN(VisionPanel::SaveChangedProperties));
}

void VisionPanel::UpdateCameraProperties()
{
	ImGui::Begin("Camera Properties", nullptr, ImGuiWindowFlags_NoTabBar);

	UI::PushID();

	const int32_t cameraCount = static_cast<int32_t>(myVisionCams.size());
	if (mySelectedCamera > cameraCount)
	{
		mySelectedCamera = cameraCount - 1;
	}

	if (mySelectedCamera == -1 || !myVisionCams[mySelectedCamera].IsValid())
	{
		UI::PopID();
		ImGui::End();
		return;
	}

	Volt::Entity selectedCam = myVisionCams[mySelectedCamera];
	auto& baseCamComp = selectedCam.GetComponent<Volt::CameraComponent>();
	auto& visionCamComp = selectedCam.GetComponent<Volt::VisionCameraComponent>();

	std::string labelName = "Cam: " + selectedCam.GetComponent<Volt::TagComponent>().tag + " [" + std::to_string(mySelectedCamera) + "]";;
	ImGui::LabelText("##CamName", labelName.c_str());

	ImGui::SameLine(0, -1000);
	if (ImGui::Button("Set As Default"))
	{
		for (auto& cam : myVisionCams)
		{
			cam.GetComponent<Volt::VisionCameraComponent>().isDefault = false;
		}

		visionCamComp.isDefault = true;
	}

	ImGui::Spacing();

	ImGui::Separator();

	ImGui::LabelText("", "Core Settings");
	if (UI::BeginProperties("Core Settings"))
	{
		{
			const Volt::IEnumTypeDesc* camTypeDesc = Volt::GetTypeDesc<Volt::eCameraType>();
			UI::ComboProperty("Camera Type", *(int32_t*)&visionCamComp.cameraType, camTypeDesc->GetConstantNames());
		}

		{
			const Volt::IEnumTypeDesc* camTypeDesc = Volt::GetTypeDesc<Volt::eBlendType>();
			UI::ComboProperty("Blend Type", *(int32_t*)&visionCamComp.blendType, camTypeDesc->GetConstantNames());
		}

		if (visionCamComp.blendType != Volt::eBlendType::None)
		{
			ImGui::TableNextColumn();
			ImGui::TextUnformatted("Blend Time");

			ImGui::TableNextColumn();
			ImGui::PushItemWidth(ImGui::GetColumnWidth());

			ImGui::DragFloat("##", &visionCamComp.blendTime);

			UI::Property("Additive Blend", visionCamComp.additiveBlend);
		}

		ImGui::TableNextColumn();
		ImGui::TextUnformatted("FOV");

		ImGui::TableNextColumn();
		ImGui::PushItemWidth(ImGui::GetColumnWidth());

		ImGui::DragFloat("##Fov", &baseCamComp.fieldOfView);

		UI::EndProperties();
	}

	ImGui::Separator();

	ImGui::LabelText("", "Transform");
	if (UI::BeginProperties("Transform"))
	{
		if (UI::PropertyEntity("Follow", myCurrentScene, visionCamComp.followId, "Camera follows this entity"))
		{
			Volt::Entity followEnt = myCurrentScene->GetEntityFromUUID(visionCamComp.followId);

			visionCamComp.offset = myVisionCams[mySelectedCamera].GetPosition() - followEnt.GetPosition();
		}

		UI::PropertyEntity("LookAt", myCurrentScene, visionCamComp.lookAtId, "Camera looks at this entity");

		if (visionCamComp.followId != Volt::Entity::NullID())
		{
			ImGui::LabelText("##Locks", "Constraints");

			ImGui::Checkbox("X", &visionCamComp.xFollowLock);
			ImGui::SameLine();
			ImGui::Checkbox("Y", &visionCamComp.yFollowLock);
			ImGui::SameLine();
			ImGui::Checkbox("Z", &visionCamComp.zFollowLock);

			UI::Property("Offset", visionCamComp.offset);
			UI::Property("Damp Amount", visionCamComp.damping);

			if (visionCamComp.damping > 0)
			{
				UI::Property("X", visionCamComp.xShouldDamp);
				ImGui::SameLine();
				UI::Property("Y", visionCamComp.yShouldDamp);
				ImGui::SameLine();
				UI::Property("Z", visionCamComp.zShouldDamp);
			}
		}

		UI::EndProperties();
	}

	ImGui::Separator();

	ImGui::LabelText("", "Controller");

	if (UI::BeginProperties("Controller"))
	{
		if (visionCamComp.cameraType == Volt::eCameraType::FirstPerson || visionCamComp.cameraType == Volt::eCameraType::ThirdPerson)
		{
			UI::Property("Mouse Sensitivity", visionCamComp.mouseSensitivity);
			UI::Property("Focal Distance", visionCamComp.focalDistance);
		}
		UI::EndProperties();
	}

	if (visionCamComp.cameraType == Volt::eCameraType::ThirdPerson)
	{
		ImGui::Separator();

		ImGui::LabelText("", "Collision");

		if (UI::BeginProperties("Collision"))
		{
			UI::Property("Use Collision", visionCamComp.isColliding);
			if (visionCamComp.isColliding)
			{
				UI::PropertyEntity("Focus Point", myCurrentScene, visionCamComp.collisionRayPoint, "Tries to not hide this entity, will be set to the follow entity if left empty");

				UI::Property("Collision Radius", visionCamComp.collisionRadius);

				if (ImGui::Button("+"))
				{
					visionCamComp.layerMasks.push_back(0);
				}
				UI::SameLine();
				if (ImGui::Button("-"))
				{
					visionCamComp.layerMasks.erase(visionCamComp.layerMasks.begin() + visionCamComp.layerMasks.size() - 1);
				}

				UI::SameLine();

				if (ImGui::CollapsingHeader("Layer Masks", ImGuiTreeNodeFlags_DefaultOpen))
				{
					if (!visionCamComp.layerMasks.empty())
					{
						for (int i = 0; i < visionCamComp.layerMasks.size(); i++)
						{
							UI::Property("Layer", visionCamComp.layerMasks[i]);
						}
					}
				}
			}
			UI::EndProperties();
		}
	}

	UI::PopID();

	ImGui::End();
}

void VisionPanel::UpdateSetDetails()
{
	ImGui::Begin("Studio", nullptr, ImGuiWindowFlags_NoTabBar);

	ImGui::LabelText("##Cams", "Cameras On Set");

	ImGui::Separator();

	ImGui::Spacing();

	if (myVisionCams.empty())
	{
		ImGui::End();
		return;
	}

	static int selected = -1;
	for (int n = 0; n < myVisionCams.size(); n++)
	{
		if (myVisionCams[n].IsValid())
		{
			auto& tagComp = myVisionCams[n].GetComponent<Volt::TagComponent>();

			char buf[32];

			std::string camName = tagComp.tag + " [" + std::to_string(n) + "]";

			if (myVisionCams[n].GetComponent<Volt::VisionCameraComponent>().isDefault)
				camName += " (default)";

			sprintf_s(buf, camName.c_str(), n);

			if (ImGui::Selectable(buf, selected == n))
			{
				selected = n;
			}
			ImGui::SeparatorWidth(80);
		}
		else
		{
			myVisionCams.erase(myVisionCams.begin() + n);
		}

	}

	if (selected >= 0)
	{
		if (selected != mySelectedCamera)
		{
			for (auto& cam : myVisionCams)
			{
				cam.GetComponent<Volt::CameraComponent>().priority = 0;
			}

			myVisionCams[selected].GetComponent<Volt::CameraComponent>().priority = 1;
		}

		mySelectedCamera = selected;
	}

	ImGui::End();
}

bool VisionPanel::RelocateSetOnLoad(Volt::OnSceneLoadedEvent& e)
{
	myCameraTriggers.clear();
	myCameraTriggers = GetAllCameraTriggers();

	myVisionCams.clear();
	myVisionCams = GetAllCameras();

	if (myNewSettingsSaved && e.GetScene() == myCurrentScene)
	{
		for (size_t i = 0; i < myVisionCams.size(); i++)
		{
			myVisionCams[i].GetComponent<Volt::CameraComponent>() = myVTCamComponents[i].first;
			myVisionCams[i].GetComponent<Volt::VisionCameraComponent>() = myVTCamComponents[i].second;
		}

		myNewSettingsSaved = false;
	}

	return false;
}

bool VisionPanel::SaveChangedProperties(Volt::OnSceneStopEvent& e)
{
	myVTCamComponents.clear();
	for (auto& cam : myVisionCams)
	{
		myVTCamComponents.emplace_back(std::pair(cam.GetComponent<Volt::CameraComponent>(), cam.GetComponent<Volt::VisionCameraComponent>()));
	}
	myNewSettingsSaved = true;

	return false;
}

void VisionPanel::InitCinema()
{
	if (myVisionCams.empty())
	{
		myVisionCams.emplace_back(CreateNewCamera());
	}
}

void VisionPanel::GetCinema()
{
	myVisionCams.clear();
	myVisionCams = GetAllCameras();
}

Volt::Entity VisionPanel::CreateNewTrigger()
{
	Volt::Entity triggerEnt = myCurrentScene->CreateEntity();

	auto& tagComp = triggerEnt.GetComponent<Volt::TagComponent>();
	tagComp.tag = "CameraTriggerBox " + std::to_string(myCameraTriggers.size());

	triggerEnt.AddComponent<Volt::BoxColliderComponent>();

	return triggerEnt;
}

const Vector<Volt::Entity> VisionPanel::GetAllCameraTriggers()
{
	Vector<Volt::Entity> triggerIDs = myCurrentScene->GetAllEntitiesWith<Volt::VisionCameraComponent>();

	if (triggerIDs.empty())
	{
		VT_LOG(LogSeverity::Error, "VTCINEMA: No triggers loacated!");
		return Vector<Volt::Entity>();
	}

	Vector<Volt::Entity> triggers;
	triggers.reserve(triggerIDs.size());

	for (auto& id : triggerIDs)
	{
		triggers.emplace_back(id);
	}

	return triggers;
}

const Vector<Volt::Entity> VisionPanel::GetAllCameras()
{
	Vector<Volt::Entity> cameraIDs = myCurrentScene->GetAllEntitiesWith<Volt::VisionCameraComponent>();

	if (cameraIDs.empty())
	{
		VT_LOG(LogSeverity::Error, "VTCINEMA: No cameras loacated!");
		return Vector<Volt::Entity>();
	}

	Vector<Volt::Entity> vtCams;
	vtCams.reserve(cameraIDs.size());

	for (auto& id : cameraIDs)
	{
		vtCams.emplace_back(id);
	}

	return vtCams;
}

void VisionPanel::UpdateSelectedCamera()
{
	if (mySelectedCamera == -1 || !myVisionCams[mySelectedCamera].IsValid())
	{
		return;
	}

	Volt::Entity selectedEnt = myVisionCams[mySelectedCamera];
	auto& visionCamComp = selectedEnt.GetComponent<Volt::VisionCameraComponent>();

	if (visionCamComp.followId != Volt::Entity::NullID())
	{
		Volt::Entity target = myCurrentScene->GetEntityFromUUID(visionCamComp.followId);
		selectedEnt.SetPosition(target.GetPosition() + visionCamComp.offset);
	}

	if (visionCamComp.lookAtId != Volt::Entity::NullID())
	{
		Volt::Entity lookAtEnt = myCurrentScene->GetEntityFromUUID(visionCamComp.lookAtId);
		glm::vec3 lookAtPos = lookAtEnt.GetPosition();

		selectedEnt.SetLocalRotation(glm::quatLookAtLH(glm::normalize(lookAtPos - selectedEnt.GetPosition()), { 0,1,0 }));
	}
}

Volt::Entity VisionPanel::CreateNewCamera()
{
	Volt::Entity camEnt = myCurrentScene->CreateEntity();

	auto& tagComp = camEnt.GetComponent<Volt::TagComponent>();
	tagComp.tag = "VCam" + std::to_string(myVisionCams.size());

	auto& visionCamComp = camEnt.AddComponent<Volt::VisionCameraComponent>();

	auto& baseCamComp = camEnt.AddComponent<Volt::CameraComponent>();
	baseCamComp.fieldOfView = visionCamComp.fieldOfView;

	camEnt.SetPosition(myEditorCamera->GetCamera()->GetPosition());
	camEnt.SetLocalRotation(myEditorCamera->GetCamera()->GetRotation());

	return camEnt;
}
