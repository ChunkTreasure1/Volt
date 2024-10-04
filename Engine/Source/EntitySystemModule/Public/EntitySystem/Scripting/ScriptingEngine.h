#pragma once

#include "EntitySystem/Config.h"

#include <CoreUtilities/Core.h>

class ECSEventDispatcher;

class VTES_API ScriptingEngine
{
public:
	ScriptingEngine();
	~ScriptingEngine();

	ECSEventDispatcher& GetEventDispatcher() { return *m_ecsEventDispatcher; }

private:
	Scope<ECSEventDispatcher> m_ecsEventDispatcher;
};
