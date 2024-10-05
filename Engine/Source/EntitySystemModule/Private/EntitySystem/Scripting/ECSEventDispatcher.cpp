#include "espch.h"

#include "EntitySystem/Scripting/ECSEventDispatcher.h"

ECSEventDispatcher::ECSEventDispatcher()
{
}

ECSEventDispatcher::~ECSEventDispatcher()
{
}

void ECSEventDispatcher::OnRuntimeStart()
{
	m_isInRuntime = true;
}

void ECSEventDispatcher::OnRuntimeEnd()
{
	m_isInRuntime = false;
}

void ECSEventDispatcher::Clear()
{
	m_registeredListeners.clear();
}
