#include "gepch.h"
#include "TestController.h"

#include <Volt/Input/Input.h>
#include <Volt/Input/KeyCodes.h>
#include <Volt/VTCinema/Director.h>

VT_REGISTER_SCRIPT(TestController)

TestController::TestController(const Volt::Entity& aEntity)
	:	ScriptBase(aEntity)
{
}

void TestController::OnAwake()
{
	myMoveSpeed = 300.f;
}

void TestController::OnUpdate(float aDeltaTime)
{
	if (Volt::Input::IsKeyDown(VT_KEY_W)) 
	{
		myEntity.SetPosition(myEntity.GetPosition() + myEntity.GetForward() * myMoveSpeed * aDeltaTime);
	}

	if (Volt::Input::IsKeyDown(VT_KEY_S))
	{
		myEntity.SetPosition(myEntity.GetPosition() - myEntity.GetForward() * myMoveSpeed * aDeltaTime);
	}

	if (Volt::Input::IsKeyDown(VT_KEY_D))
	{
		myEntity.SetPosition(myEntity.GetPosition() + myEntity.GetRight() * myMoveSpeed * aDeltaTime);
	}

	if (Volt::Input::IsKeyDown(VT_KEY_A))
	{
		myEntity.SetPosition(myEntity.GetPosition() - myEntity.GetRight() * myMoveSpeed * aDeltaTime);
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
		Director::Get().SetActiveCamera(2);
	}
}
