#pragma once

#include "EntitySystem/Config.h"

#include <CoreUtilities/CompilerTraits.h>
#include <CoreUtilities/Containers/Vector.h>

#include <functional>

class ECSBuilder;

class VTES_API ECSSystemRegistry
{
public:
	bool RegisterECSModule(std::function<void(ECSBuilder& builder)> func);
	void Build(ECSBuilder& builder);

private:
	Vector<std::function<void(ECSBuilder& builder)>> m_registeredModules;
};

extern VTES_API ECSSystemRegistry g_ecsSystemRegistry;

VT_INLINE ECSSystemRegistry& GetECSSystemRegistry()
{
	return g_ecsSystemRegistry;
}

#define VT_REGISTER_ECS_MODULE(func) inline static bool func ## _registered = ::GetECSSystemRegistry().RegisterECSModule(func)
