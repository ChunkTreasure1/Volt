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
		myActiveCamera = myVTCams[0];
	}

	myActiveCamera.GetComponent<Volt::CameraComponent>().priority = 1;
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

	myLerpTime = 2.f;
	myCurrentLerpTime = 0.f;

	myIsBlending = true;
}

Volt::Entity Director::CreateTransitionCamera()
{
	Volt::Entity cam = myEntity.GetScene()->CreateEntity();
	cam.GetComponent<Volt::TagComponent>().tag = "TransitionCamera";
	
	auto baseCamComp = myActiveCamera.GetComponent<Volt::CameraComponent>();

	cam.AddComponent<Volt::CameraComponent>();
	cam.AddComponent<Volt::VTCamComponent>();

	return cam;
}

void Director::BlendCameras(float aDeltaTime)
{
	myCurrentLerpTime += aDeltaTime;

	if (myCurrentLerpTime > myLerpTime)
	{
		myTransitionCamera.GetComponent<Volt::CameraComponent>().priority = 2;
		myActiveCamera.GetComponent<Volt::CameraComponent>().priority = 1;

		myCurrentLerpTime = 0.f;
		myIsBlending = false;

		return;
	}

	float t = myCurrentLerpTime / myLerpTime;
	t = t * t * (3.f - 2.f * t);

	myTransitionCamera.SetPosition(gem::lerp(myTransitionCamera.GetPosition(), myActiveCamera.GetPosition(), t));
	myTransitionCamera.SetRotation(gem::lerp(myTransitionCamera.GetRotation(), myActiveCamera.GetRotation(), t));
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
