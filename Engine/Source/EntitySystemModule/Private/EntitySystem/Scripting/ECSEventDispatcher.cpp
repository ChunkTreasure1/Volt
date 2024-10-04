#include "espch.h"

#include "EntitySystem/Scripting/ECSEventDispatcher.h"

ECSEventDispatcher::ECSEventDispatcher()
{
}

void ECSEventDispatcher::Clear()
{
	m_registeredListeners.clear();
}
