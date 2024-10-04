#include "espch.h"

#include "EntitySystem/Scripting/ScriptingEngine.h"
#include "EntitySystem/Scripting/ECSEventDispatcher.h"

ScriptingEngine::ScriptingEngine()
{
	m_ecsEventDispatcher = CreateScope<ECSEventDispatcher>();
}

ScriptingEngine::~ScriptingEngine()
{
	m_ecsEventDispatcher = nullptr;
}
