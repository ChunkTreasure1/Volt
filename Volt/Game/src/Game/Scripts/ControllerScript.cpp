#include "gepch.h"
#include "ControllerScript.h"

#include <Volt/Components/PhysicsComponents.h>
#include <Volt/Components/Components.h>
#include <Volt/Components/NavigationComponents.h>

#include <Volt/Asset/AssetManager.h>
#include <Volt/Input/KeyCodes.h>

#include <Volt/Utility/Random.h>

#include <Volt/Core/Application.h>
#include <Volt/Events/MouseEvent.h>

ControllerScript::ControllerScript(Volt::Entity entity)
	: Volt::ScriptBase(entity)
{

}

void ControllerScript::OnStart()
{

}

void ControllerScript::OnEvent(Volt::Event& e)
{
	Volt::EventDispatcher dispatcher(e);
	dispatcher.Dispatch<Volt::KeyPressedEvent>(VT_BIND_EVENT_FN(ControllerScript::OnKeyPressedEvent));

	if (Volt::Application::Get().IsRuntime())
	{
		dispatcher.Dispatch<Volt::MouseMovedEvent>([&](Volt::MouseMovedEvent& e)
			{
				myViewportMousePos = { e.GetX(), e.GetY() };

		return false;
			});
	}
	else
	{
		dispatcher.Dispatch<Volt::MouseMovedViewportEvent>([&](Volt::MouseMovedViewportEvent& e)
			{
				myViewportMousePos = { e.GetX(), e.GetY() };
		return false;
			});
	}

	dispatcher.Dispatch<Volt::ViewportResizeEvent>([&](Volt::ViewportResizeEvent& e)
		{
			myViewportSize = { (float)e.GetWidth(), (float)e.GetHeight() };
	return false;
		});
}

void ControllerScript::OnUpdate(float aDeltaTime)
{
	if (myEntity.HasComponent<Volt::AgentComponent>())
	{
		//auto force = Volt::SteeringBehavior::Seek(myEntity, myTarget);
		auto force = Volt::SteeringBehavior::Wander(myEntity, myDirection, 100.f, 200.f);
		//force += Volt::SteeringBehavior::Separation(myEntity, 100.f);
		myEntity.GetComponent<Volt::AgentComponent>().steeringForce = force;
	}
}

gem::vec3 ControllerScript::GetWorldPosFromMouse()
{
	Volt::Entity cam = GetCameraEntity();
	gem::vec3 dir;

	if (cam.HasComponent<Volt::CameraComponent>())
	{
		dir = cam.GetComponent<Volt::CameraComponent>().camera->ScreenToWorldCoords(myViewportMousePos, myViewportSize);
	}

	gem::vec3 camPos = cam.GetPosition();
	gem::vec3 targetPos;
	targetPos = camPos;

	while (targetPos.y > myEntity.GetPosition().y)
	{
		gem::vec3 deltaPos = targetPos;
		if (deltaPos.y > (targetPos + dir).y)
		{
			targetPos += dir;
			continue;
		}
		break;
	}

	return { targetPos.x, 0.f, targetPos.z };
}

Volt::Entity ControllerScript::GetCameraEntity()
{
	Volt::Entity tempEnt;

	myEntity.GetScene()->GetRegistry().ForEach<Volt::CameraComponent>([&](Wire::EntityId id, const Volt::CameraComponent& scriptComp)
		{
			tempEnt = { id, myEntity.GetScene() };
	return;
		});

	return tempEnt;
}

bool ControllerScript::OnKeyPressedEvent(Volt::KeyPressedEvent& e)
{
	switch (e.GetKeyCode())
	{
	case VT_KEY_SPACE:
	{
  		myTarget = GetWorldPosFromMouse();

		if (myEntity.HasComponent<Volt::AgentComponent>())
		{
			myEntity.GetComponent<Volt::AgentComponent>().target = myTarget;
			myDirection = myTarget - myEntity.GetWorldPosition();

			std::stringstream ss;
			ss << "TARGET: " << myTarget.x << ", " << myTarget.y << ", " << myTarget.z;

			VT_INFO(ss.str());
		}
		break;
	}
	}

	return false;
}

VT_REGISTER_SCRIPT(ControllerScript);