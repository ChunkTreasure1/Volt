#include "gepch.h"
#include "SeekScript.h"

#include <Volt/Components/PhysicsComponents.h>
#include <Volt/Components/Components.h>
#include <Volt/Components/NavigationComponents.h>

#include <Volt/Components/GameComponents.h>

#include <Volt/Scene/Scene.h>
#include <Volt/Utility/Random.h>

#include <Volt/Core/Application.h>
#include <Volt/Events/MouseEvent.h>

SeekScript::SeekScript(Volt::Entity entity)
	: Volt::ScriptBase(entity)
{

}

void SeekScript::OnStart()
{

}

void SeekScript::OnEvent(Volt::Event& e)
{
	Volt::EventDispatcher dispatcher(e);
}

void SeekScript::OnUpdate(float aDeltaTime)
{
	myEntity.GetScene()->GetRegistry().ForEach<WandererComponent>([&](Wire::EntityId id, WandererComponent& comp)
		{
			Volt::Entity e(id, myEntity.GetScene());
			myTarget = e.GetWorldPosition();
		});

	if (myEntity.HasComponent<Volt::AgentComponent>())
	{
		auto force = Volt::SteeringBehavior::Seek(myEntity, myTarget);
		myEntity.GetComponent<Volt::AgentComponent>().steeringForce = force;
	}
}

VT_REGISTER_SCRIPT(SeekScript);