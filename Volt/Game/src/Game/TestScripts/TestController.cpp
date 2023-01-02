#include "gepch.h"
#include "TestController.h"

#include <Volt/Input/Input.h>
#include <Volt/Input/KeyCodes.h>
#include <Volt/VTCinema/Director.h>
#include <Volt/Components/VTCinemaComponents.h>

VT_REGISTER_SCRIPT(TestController)

TestController::TestController(const Volt::Entity& aEntity)
	:	Script(aEntity)
{
}

void TestController::DoCameraSequence()
{
	Director::Get().SetActiveCamera(myCamIndex);

	if (Director::Get().IsCameraActive(myCamIndex))
	{
		myCamIndex++;
		if (myCamIndex >= Director::Get().GetAllCamerasInScene().size())
		{
			myCamIndex = 0;
		}
	}
}

void TestController::OnAwake()
{
	Volt::Entity aimCam = Volt::Entity{ myEntity.GetComponent<Volt::TestControllerComponent>().AimCamera, myEntity.GetScene() };

	if (aimCam)
	{
		myLookCamera = aimCam;
	}

	myCamIndex = 0;
	myMoveSpeed = 700.f;
}

void TestController::OnUpdate(float aDeltaTime)
{
	if (myCameraSequenceOn)
	{
		DoCameraSequence();
	}

	if (Volt::Input::IsKeyDown(VT_KEY_W)) 
	{
		gem::vec3 walkDir = { myLookCamera.GetForward().x, 0, myLookCamera.GetForward().z };
		myEntity.SetPosition(myEntity.GetPosition() + gem::normalize(walkDir) * myMoveSpeed * aDeltaTime);
	}

	if (Volt::Input::IsKeyDown(VT_KEY_S))
	{
		gem::vec3 walkDir = { myLookCamera.GetForward().x, 0, myLookCamera.GetForward().z };
		myEntity.SetPosition(myEntity.GetPosition() - gem::normalize(walkDir) * myMoveSpeed * aDeltaTime);
	}

	if (Volt::Input::IsKeyDown(VT_KEY_D))
	{
		gem::vec3 walkDir = { myLookCamera.GetRight().x, 0, myLookCamera.GetRight().z };
		myEntity.SetPosition(myEntity.GetPosition() + gem::normalize(walkDir) * myMoveSpeed * aDeltaTime);
	}

	if (Volt::Input::IsKeyDown(VT_KEY_A))
	{
		gem::vec3 walkDir = { myLookCamera.GetRight().x, 0, myLookCamera.GetRight().z };
		myEntity.SetPosition(myEntity.GetPosition() - gem::normalize(walkDir) * myMoveSpeed * aDeltaTime);
	}

	if (Volt::Input::IsKeyDown(VT_KEY_B))
	{
		Director::Get().SetActiveCamera(1);
	}

	if (Volt::Input::IsKeyDown(VT_KEY_V))
	{
		Director::Get().SetActiveCamera(0);
	}

	if (Volt::Input::IsKeyDown(VT_KEY_N))
	{
		myCameraSequenceOn = true;
	}

	if (Volt::Input::IsMouseButtonPressed(1))
	{
		Director::Get().SetActiveCamera(0);
	}
	else
	{
		Director::Get().SetActiveCamera(1);
	}

}
