#include "espch.h"

#include "EntitySystem/Scripting/ScriptingEngine.h"
#include "EntitySystem/Scripting/ECSEventDispatcher.h"

#include "EntitySystem/Scripting/ECSSystemRegistry.h"
#include "EntitySystem/ComponentRegistry.h"

ScriptingEngine::ScriptingEngine()
{
	VT_ENSURE(s_instance == nullptr);

	m_ecsEventDispatcher = CreateScope<ECSEventDispatcher>();

	s_instance = this;
}

ScriptingEngine::~ScriptingEngine()
{
	s_instance = nullptr;

	m_ecsEventDispatcher = nullptr;

	// #TODO_Ivar: We probably don't want to clear the registry like this.
	// We probably would want the systems to unregister themselves somehow. Same with components.
	GetECSSystemRegistry().ClearRegistry();
	GetComponentRegistry().ClearRegistry();
}
