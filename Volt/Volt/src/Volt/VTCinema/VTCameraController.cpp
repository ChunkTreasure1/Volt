#include "vtpch.h"
#include "VTCameraController.h"

#include <Volt/Components/Components.h>
#include <Volt/Components/VTCinemaComponents.h>
#include <Volt/Input/Input.h>
#include <Volt/Utility/UIUtility.h>
#include <Volt/Physics/Physics.h>
#include <Volt/Physics/PhysicsScene.h>

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
		myMaxFocalDist = vtCamComp.focalDistance;
		myCurrentFocalDist = myMaxFocalDist;
		myCurrentLerpTime = myLerpTime;
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

	const float yawSign = myEntity.GetUp().y < 0.f ? -1.f : 1.f;
	myYawDelta = yawSign * deltaPos.x * vtCamComp.mouseSensitivity;
	myPitchDelta = deltaPos.y * vtCamComp.mouseSensitivity;

	myRotation += gem::radians(gem::vec3{ myPitchDelta, myYawDelta, 0.f });

	Volt::Entity target = Volt::Entity{ vtCamComp.followId, myEntity.GetScene() };

	if (target) 
	{
		myEntity.SetLocalRotation(gem::quat(myRotation));

		const gem::vec3 focalPoint = target.GetPosition() + vtCamComp.offset;
		const gem::vec3 dir = myEntity.GetPosition() - focalPoint;
		gem::vec3 position = { 0 };

		if (vtCamComp.isColliding) 
		{
			Volt::RaycastHit hit;
			if (Volt::Physics::GetScene()->Raycast(focalPoint, gem::normalize(dir), vtCamComp.focalDistance, &hit)) 
			{
				myCurrentLerpTime = 0.f;
				myCurrentFocalDist = gem::length(gem::normalize(dir) * hit.distance) - vtCamComp.collisionRadius;
				myLastHitFocalDist = myCurrentFocalDist;
			}
			else 
			{
				myCurrentLerpTime += aDeltaTime;
				if (myCurrentLerpTime >= myLerpTime)
				{
					myCurrentLerpTime = myLerpTime;
				}

				float t = myCurrentLerpTime / myLerpTime;

				t = gem::sin(t * gem::pi() * 0.5f); //Ease Out
				myCurrentFocalDist = gem::lerp(myLastHitFocalDist, myMaxFocalDist, t);
			}
		}

		position = focalPoint - myEntity.GetForward() * myCurrentFocalDist;

		position.y = gem::clamp(position.y, focalPoint.y - (vtCamComp.focalDistance + 50), focalPoint.y + (vtCamComp.focalDistance - 50));

		myEntity.SetPosition(position);
	}

	myLastMousePos = mousePos;
}
