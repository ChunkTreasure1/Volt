#include "espch.h"
#include "EntitySystem/Scripting/ECSSystemRegistry.h"

ECSSystemRegistry g_ecsSystemRegistry;

bool ECSSystemRegistry::RegisterECSModule(std::function<void(ECSBuilder& builder)> func)
{
	m_registeredModules.emplace_back(func);
	return true;
}

void ECSSystemRegistry::Build(ECSBuilder& builder)
{
	for (auto& module : m_registeredModules)
	{
		module(builder);
	}
}
