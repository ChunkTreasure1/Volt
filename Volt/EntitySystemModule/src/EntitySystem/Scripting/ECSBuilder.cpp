#include "espch.h"
#include "ECSBuilder.h"

ECSGameLoopContainer& ECSBuilder::GetGameLoop(GameLoop gameLoopType)
{
	ECSGameLoopContainer container;
	m_gameLoops[gameLoopType] = container;
	return m_gameLoops[gameLoopType];
}

void ECSGameLoopContainer::Execute(float deltaTime)
{
	for (auto& [id, system] : m_registeredSystems)
	{
		system.systemFunc(*m_registry, deltaTime);
	}
}
