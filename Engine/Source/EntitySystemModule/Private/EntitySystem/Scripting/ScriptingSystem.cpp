#include "espch.h"
#include "EntitySystem/Scripting/ScriptingSystem.h"

#include "EntitySystem/Scripting/ECSSystemRegistry.h"
#include "EntitySystem/ComponentRegistry.h"

ScriptingSystem::ScriptingSystem()
{
}

ScriptingSystem::~ScriptingSystem()
{
	// #TODO_Ivar: We probably don't want to clear the registry like this.
	// We probably would want the systems to unregister themselves somehow. Same with components.
	GetECSSystemRegistry().ClearRegistry();
	GetComponentRegistry().ClearRegistry();
}
