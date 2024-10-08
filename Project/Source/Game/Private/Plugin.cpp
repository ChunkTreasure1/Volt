#include "Plugin.h"

#include <LogModule/Log.h>

#include <EntitySystem/Scripting/ECSAccessBuilder.h>
#include <EntitySystem/Scripting/ECSBuilder.h>
#include <EntitySystem/Scripting/ECSSystemRegistry.h>
#include <EntitySystem/Scripting/ECSEventDispatcher.h>

#include <EntitySystem/Scripting/ScriptingEngine.h>

#include <Volt/Physics/PhysicsEvents.h>

#include <InputModule/Input.h>

void GamePlugin::Initialize()
{
	VT_LOG(Trace, "Hello from GamePlugin!");
}

void GamePlugin::Shutdown()
{
}

using PlayerEntity = ECS::Access
	::Write<Volt::TransformComponent>
	::With<PlayerComponent>
	::As<ECS::Type::Entity>;

void PlayerSystem(PlayerEntity entity, float deltaTime)
{
	constexpr float speed = 100.f;

	if (Volt::Input::IsButtonDown(Volt::InputCode::W))
	{
		entity.SetPosition(entity.GetPosition() + entity.GetForward() * speed * deltaTime);
	}

	if (Volt::Input::IsButtonDown(Volt::InputCode::S))
	{
		entity.SetPosition(entity.GetPosition() - entity.GetForward() * speed * deltaTime);
	}

	if (Volt::Input::IsButtonDown(Volt::InputCode::A))
	{
		entity.SetPosition(entity.GetPosition() - entity.GetRight() * speed * deltaTime);
	}

	if (Volt::Input::IsButtonDown(Volt::InputCode::D))
	{
		entity.SetPosition(entity.GetPosition() + entity.GetRight() * speed * deltaTime);
	}
}

void OnPlayerCollisionEnterEvent(Volt::OnCollisionEnterEvent& enterEvent)
{
	VT_ENSURE(enterEvent.GetEntityA().HasComponent<PlayerComponent>() || enterEvent.GetEntityB().HasComponent<PlayerComponent>());

	VT_LOG(Trace, "Player Collision!");
}

void RegisterModule(ECSBuilder& builder)
{
	builder.GetGameLoop(GameLoop::Variable).RegisterSystem(PlayerSystem);

	builder.RegisterListenerSystem<Volt::OnCollisionEnterEvent, PlayerEntity>(OnPlayerCollisionEnterEvent);
}

VT_REGISTER_ECS_MODULE(RegisterModule);