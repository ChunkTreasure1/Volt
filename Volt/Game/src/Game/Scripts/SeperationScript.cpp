#include "gepch.h"
#include "SeperationScript.h"

#include <Volt/Components/PhysicsComponents.h>
#include <Volt/Components/Components.h>
#include <Volt/Components/NavigationComponents.h>

#include <Volt/Asset/AssetManager.h>
#include <Volt/Input/KeyCodes.h>

#include <Volt/Utility/Random.h>

#include <Volt/Core/Application.h>
#include <Volt/Events/MouseEvent.h>

SeperationScript::SeperationScript(Volt::Entity entity)
	: Volt::ScriptBase(entity)
{

}

void SeperationScript::OnStart()
{

}

void SeperationScript::OnEvent(Volt::Event& e)
{
	Volt::EventDispatcher dispatcher(e);
}

void SeperationScript::OnUpdate(float aDeltaTime)
{
	if (myEntity.HasComponent<Volt::AgentComponent>())
	{
		auto force = Volt::SteeringBehavior::Separation(myEntity, 250.f);
		myEntity.GetComponent<Volt::AgentComponent>().steeringForce = force;
	}
}

VT_REGISTER_SCRIPT(SeperationScript);