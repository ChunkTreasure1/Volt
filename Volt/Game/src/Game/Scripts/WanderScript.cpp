#include "gepch.h"
#include "WanderScript.h"

#include <Volt/Components/PhysicsComponents.h>
#include <Volt/Components/Components.h>
#include <Volt/Components/NavigationComponents.h>

#include <Volt/Asset/AssetManager.h>
#include <Volt/Input/KeyCodes.h>

#include <Volt/Utility/Random.h>

#include <Volt/Core/Application.h>
#include <Volt/Events/MouseEvent.h>

WanderScript::WanderScript(Volt::Entity entity)
	: Volt::ScriptBase(entity)
{

}

void WanderScript::OnStart()
{
	myDirection.x = Volt::Random::Float(-1.f, 1.f);
	myDirection.y = 0.f;
	myDirection.z = Volt::Random::Float(-1.f, 1.f);
}

void WanderScript::OnEvent(Volt::Event& e)
{
	Volt::EventDispatcher dispatcher(e);
}

void WanderScript::OnUpdate(float aDeltaTime)
{
	static float updateDirectionTimer = 10.f;

	updateDirectionTimer -= aDeltaTime;
	if (updateDirectionTimer < 0.f)
	{
		myDirection.x = Volt::Random::Float(-1.f, 1.f);
		myDirection.y = 0.f;
		myDirection.z = Volt::Random::Float(-1.f, 1.f);
		updateDirectionTimer = 10.f;
	}

	auto pos = myEntity.GetWorldPosition();
	if (pos.x > 1000.f)
	{
		myDirection.x = -1.f;
	}
	if (pos.x < -1000.f)
	{
		myDirection.x = 1.f;
	}
	if (pos.z > 1000.f)
	{
		myDirection.z = -1.f;
	}
	if (pos.z < -1000.f)
	{
		myDirection.z = 1.f;
	}

	if (myEntity.HasComponent<Volt::AgentComponent>())
	{
		auto force = Volt::SteeringBehavior::Wander(myEntity, myDirection, 100.f, 200.f);
		myEntity.GetComponent<Volt::AgentComponent>().steeringForce = force;
	}
}

VT_REGISTER_SCRIPT(WanderScript);