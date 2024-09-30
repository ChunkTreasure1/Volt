#pragma once

#include "EntitySystem/Config.h"

#include <CoreUtilities/Core.h>

class ECSEventDispatcher;

class VTES_API ScriptingEngine
{
public:
	ScriptingEngine();
	~ScriptingEngine();

	static ECSEventDispatcher& GetEventDispatcher() { return *s_instance->m_ecsEventDispatcher; }

private:
	inline static ScriptingEngine* s_instance = nullptr;

	Scope<ECSEventDispatcher> m_ecsEventDispatcher;
};
