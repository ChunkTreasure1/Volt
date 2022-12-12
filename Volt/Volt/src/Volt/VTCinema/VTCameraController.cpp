#include "vtpch.h"
#include "VTCameraController.h"

#include <Volt/Components/Components.h>
#include <Volt/Components/VTCinemaComponents.h>
#include <Volt/Input/Input.h>

VT_REGISTER_SCRIPT(VTCameraController);

VTCameraController::VTCameraController(const Volt::Entity& aEntity)
	:ScriptBase(aEntity)
{
}

void VTCameraController::OnAwake()
{
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

		gem::vec3 lookAtPos = lookAtEnt.GetWorldPosition();
		gem::mat test = gem::lookAtLH(myEntity.GetPosition(), lookAtPos, { 0,1,0 });
		gem::vec3 rot = 0;
		gem::vec3 dump = 0;
		gem::decompose(test, dump, rot, dump);

		myEntity.SetRotation(rot * -1);
	}
}

void VTCameraController::TPSController(float aDeltaTime)
{
	const gem::vec2 mousePos = { Volt::Input::GetMouseX(), Volt::Input::GetMouseY() };
	const gem::vec2 deltaPos = (mousePos - myLastMousePos);

	/*const float yawSign = myEntity.GetUp().y < 0.f ? -1.f : 1.f;
	m_yawDelta += yawSign * deltaPos.x * m_sensitivity;
	m_pitchDelta += deltaPos.y * m_sensitivity;*/

}
