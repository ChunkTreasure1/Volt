#include "vtpch.h"
#include "Director.h"

#include <Volt/Components/Components.h>
#include <Volt/Components/VTCinemaComponents.h>

VT_REGISTER_SCRIPT(Director)

Director::Director(const Volt::Entity& aEntity)
	:ScriptBase(aEntity)
{
	if (!myInstance)
	{
		myInstance = this;
	}
}

void Director::OnAwake()
{
	std::vector<Wire::EntityId> camEntIDs = myEntity.GetScene()->GetAllEntitiesWith<Volt::VTCamComponent>();

	for (auto ID : camEntIDs)
	{
		Volt::Entity ent = { ID, myEntity.GetScene() };

		ent.GetComponent<Volt::CameraComponent>().priority = 2;

		if (ent.GetComponent<Volt::VTCamComponent>().isDefault)
			myActiveCamera = ent;
		
		myVTCams.emplace_back(ent);
	}

	if (!myActiveCamera)
	{
		if (!myVTCams.empty()) 
		{
			myActiveCamera = myVTCams[0];
			myActiveCamera.GetComponent<Volt::CameraComponent>().priority = 1;
		}
	}
}

void Director::OnUpdate(float aDeltaTime)
{
	if (myActiveCamera)
	{
		myEntity.GetComponent<Volt::DirectorComponent>().activeCam = myActiveCamera.GetId();
	}
	
	if (myIsBlending)
	{
		BlendCameras(aDeltaTime);
	}
}

void Director::InitTransitionCam()
{
	if (!myTransitionCamera)
	{
		myTransitionCamera = CreateTransitionCamera();
	}

	if (!myIsBlending)
	{
		myTransitionCamera.SetPosition(myActiveCamera.GetPosition());
		myTransitionCamera.SetRotation(myActiveCamera.GetRotation());
	}
	
	myTransitionCamera.GetComponent<Volt::CameraComponent>().priority = 0;
	myTransitionCamera.GetComponent<Volt::VTCamComponent>().blendType = myActiveCamera.GetComponent<Volt::VTCamComponent>().blendType;

	myTransitionCamStartPos = myTransitionCamera.GetPosition();
	myTransitionCamStartRot = myTransitionCamera.GetRotation();
	myTransitionCameraStartFoV = myTransitionCamera.GetComponent<Volt::CameraComponent>().fieldOfView;

	myLerpTime = myActiveCamera.GetComponent<Volt::VTCamComponent>().blendTime;
	myCurrentLerpTime = 0.f;

	myIsBlending = true;
}

Volt::Entity Director::CreateTransitionCamera()
{
	Volt::Entity cam = myEntity.GetScene()->CreateEntity();
	cam.GetComponent<Volt::TagComponent>().tag = "TransitionCamera";
	
	auto baseCamComp = myActiveCamera.GetComponent<Volt::CameraComponent>();

	auto& newBaseCamComp = cam.AddComponent<Volt::CameraComponent>();
	cam.AddComponent<Volt::VTCamComponent>();

	newBaseCamComp.fieldOfView = baseCamComp.fieldOfView;
	
	return cam;
}

void Director::BlendCameras(float aDeltaTime)
{
	auto& transitionBaseCamComp = myTransitionCamera.GetComponent<Volt::CameraComponent>();
	auto& activeBaseCamComp = myActiveCamera.GetComponent<Volt::CameraComponent>();

	auto& transitionVtCamComp = myTransitionCamera.GetComponent<Volt::VTCamComponent>();

	myCurrentLerpTime += aDeltaTime;

	if (myCurrentLerpTime >= myLerpTime)
	{
		transitionBaseCamComp.priority = 2;
		activeBaseCamComp.priority = 1;

		myCurrentLerpTime = 0.f;
		myIsBlending = false;

		return;
	}

	float t = myCurrentLerpTime / myLerpTime;

	switch (transitionVtCamComp.blendType)
	{
		case Volt::eBlendType::None:
			t = 1.f;
			myCurrentLerpTime = myLerpTime;
			break;

		case Volt::eBlendType::EaseInAndOut:
			t = t * t * t * (t * (6.f * t - 15.f) + 10.f);
			break;

		case Volt::eBlendType::EaseIn:
			t = 1.f - gem::cos(t * gem::pi() * 0.5f);
			break;

		case Volt::eBlendType::EaseOut:
			t = gem::sin(t * gem::pi() * 0.5f);
			break;

	default:
		break;
	}


	myTransitionCamera.SetPosition(gem::lerp(myTransitionCamStartPos, myActiveCamera.GetPosition(), t));
	myTransitionCamera.SetRotation(gem::lerp(myTransitionCamStartRot, myActiveCamera.GetRotation(), t));

	transitionBaseCamComp.fieldOfView = gem::lerp(myTransitionCameraStartFoV, activeBaseCamComp.fieldOfView, t);
}

void Director::SetActiveCamera(std::string aCamName)
{
	if (myActiveCamera.GetComponent<Volt::TagComponent>().tag == aCamName)
		return;

	for (auto cam : myVTCams)
	{
		if (cam.GetComponent<Volt::TagComponent>().tag == aCamName)
		{
			InitTransitionCam();

			myActiveCamera = cam;
			return;
		}
	}

	VT_CORE_ERROR(aCamName + ", does not exist!");
}

void Director::SetActiveCamera(size_t aIndex)
{
	if (aIndex + 1 > myVTCams.size() || myActiveCamera == myVTCams[aIndex])
		return;

	for (auto cam : myVTCams)
	{
		cam.GetComponent<Volt::CameraComponent>().priority = 2;
	}

	InitTransitionCam();

	myActiveCamera = myVTCams[aIndex];
}

const bool Director::IsCameraActive(size_t aIndex)
{
	if (myVTCams[aIndex].GetComponent<Volt::CameraComponent>().priority == 1)
	{
		return true;
	}
	return false;
}

const std::vector<Volt::Entity> Director::GetAllCamerasInScene()
{
	return myVTCams;
}
