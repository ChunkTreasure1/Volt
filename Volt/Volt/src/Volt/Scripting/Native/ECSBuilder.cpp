#include "vtpch.h"
#include "ECSBuilder.h"

ECSBuilder::ECSBuilder(entt::registry* registry)
	: m_registry(registry)
{
}

ECSGameLoopContainer& ECSBuilder::GetGameLoop(GameLoop gameLoopType)
{
	ECSGameLoopContainer container(m_registry);
	m_gameLoops[gameLoopType] = container;
	return m_gameLoops[gameLoopType];
}

ECSGameLoopContainer::ECSGameLoopContainer(entt::registry* registry)
	: m_registry(registry)
{
}

void ECSGameLoopContainer::Update()
{
	for (auto& [id, system] : m_registeredSystems)
	{
		system.systemFunc(*m_registry);
	}
}
