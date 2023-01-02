#include "vtpch.h"
#include "VTCameraController.h"

#include <Volt/Components/Components.h>
#include <Volt/Components/VTCinemaComponents.h>
#include <Volt/Input/Input.h>
#include <Volt/Utility/UIUtility.h>

VT_REGISTER_SCRIPT(VTCameraController);

VTCameraController::VTCameraController(const Volt::Entity& aEntity)
	:Script(aEntity)
{
}

void VTCameraController::OnAwake()
{
	auto& vtCamComp = myEntity.GetComponent<Volt::VTCamComponent>();

	if (vtCamComp.cameraType == Volt::eCameraType::ThirdPerson) 
	{
		Volt::Application::Get().GetWindow().ShowCursor(false);
		UI::SetInputEnabled(false);
	}
}

void VTCameraController::OnUpdate(float aDeltaTime)
{
	auto& vtCamComp = myEntity.GetComponent<Volt::VTCamComponent>();
	
	switch (vtCamComp.cameraType)
	{
	case Volt::eCameraType::Default:
		break;
	case Volt::eCameraType::Free:
		FreeController(aDeltaTime);
		break;
	case Volt::eCameraType::FirstPerson:
		break;
	case Volt::eCameraType::ThirdPerson:
		TPSController(aDeltaTime);
		break;
	default:
		break;
	}
}

void VTCameraController::FreeController(float aDeltaTime)
{
	auto& vtCamComp = myEntity.GetComponent<Volt::VTCamComponent>();

	if (vtCamComp.followId != 0)
	{
		Volt::Entity followEnt = Volt::Entity{ vtCamComp.followId, myEntity.GetScene() };
		myEntity.SetPosition(followEnt.GetPosition() + vtCamComp.offset);
	}

	if (vtCamComp.lookAtId != 0)
	{
		Volt::Entity lookAtEnt = Volt::Entity{ vtCamComp.lookAtId, myEntity.GetScene() };
		gem::vec3 lookAtPos = lookAtEnt.GetPosition();

		myEntity.SetLocalRotation(gem::quatLookAtLH(gem::normalize(lookAtPos - myEntity.GetPosition()), {0,1,0}));
	}
}

void VTCameraController::TPSController(float aDeltaTime)
{
	auto& vtCamComp = myEntity.GetComponent<Volt::VTCamComponent>();

	const gem::vec2 mousePos = { Volt::Input::GetMouseX(), Volt::Input::GetMouseY() };
	const gem::vec2 deltaPos = (mousePos - myLastMousePos);

	myYawDelta = 0.f;
	myPitchDelta = 0.f;

	const float yawSign = myEntity.GetUp().y < 0.f ? -1.f : 1.f;
	myYawDelta += yawSign * deltaPos.x * vtCamComp.mouseSensitivity;
	myPitchDelta += deltaPos.y * vtCamComp.mouseSensitivity;

	Volt::Entity target = Volt::Entity{ vtCamComp.followId, myEntity.GetScene() };

	if (target) 
	{
		const gem::vec3 focalPoint = target.GetPosition() + vtCamComp.offset;
		myEntity.SetLocalRotation(gem::eulerAngles(myEntity.GetLocalRotation()) + gem::radians(gem::vec3{myPitchDelta, myYawDelta, 0.f}));
		myEntity.SetPosition(focalPoint - myEntity.GetForward() * vtCamComp.focalDistance);
	}

	gem::vec3 rot = gem::eulerAngles(myEntity.GetLocalRotation());

	//if (rot.x > 180 || rot.x < -180)
	//{
	//	myEntity.SetLocalRotation(gem::vec3( { rot.x * -1, rot.y, rot.z }));
	//}

	//if (rot.y > 180 || rot.y < -180)
	//{
	//	myEntity.SetLocalRotation(gem::vec3({ rot.x, rot.y * -1, rot.z }));
	//}

	//if (rot.z > 180 || rot.z < -180)
	//{
	//	myEntity.SetLocalRotation(gem::vec3({ rot.x, rot.y, rot.z * -1 }));
	//}

	myLastMousePos = mousePos;
}
