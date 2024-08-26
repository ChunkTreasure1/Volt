#include "Plugin.h"

#include <LogModule/Log.h>

#include <EntitySystem/Scripting/ECSAccessBuilder.h>
#include <EntitySystem/Scripting/ECSBuilder.h>
#include <EntitySystem/Scripting/ECSSystemRegistry.h>

#include <Volt/Components/CoreComponents.h>

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

void PlayerSystem(PlayerEntity entity)
{
}

void RegisterModule(ECSBuilder& builder)
{
	builder.GetGameLoop(GameLoop::Variable).RegisterSystem(PlayerSystem);
}

VT_REGISTER_ECS_MODULE(RegisterModule);