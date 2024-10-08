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

void ScriptingEngine::OnRuntimeStart()
{
	m_ecsEventDispatcher->OnRuntimeStart();
}

void ScriptingEngine::OnRuntimeEnd()
{
	m_ecsEventDispatcher->OnRuntimeEnd();
}
