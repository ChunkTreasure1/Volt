#pragma once

#include "EntitySystem/Scripting/ECSSystem.h"

#include <CoreUtilities/Containers/Map.h>
#include <CoreUtilities/UUID.h>

enum class GameLoop
{
	Variable,
	Fixed
};

class ECSGameLoopContainer
{
public:
	ECSGameLoopContainer() = default;

	void VTES_API Execute(Volt::EntityScene& scene, float deltaTime);
	void VTES_API Compile();

	template<typename Ret, typename... Args>
	ECSSystem& RegisterSystem(Ret(*func)(Args...))
	{
		ECSSystemRegisterer<Ret, Args...> systemRegisterer;
		ECSSystem ecsSystem = systemRegisterer(func);

		auto id = ecsSystem.GetID();

		m_registeredSystems[id] = std::move(ecsSystem);
		return m_registeredSystems.at(id);
	}

private:
	vt::map<UUID64, ECSSystem> m_registeredSystems;
	Vector<Vector<UUID64>> m_executionBuckets;
};

class VTES_API ECSBuilder
{
public:
	ECSBuilder() = default;

	void Compile();

	ECSGameLoopContainer& GetGameLoop(GameLoop gameLoopType);

private:
	std::unordered_map<GameLoop, ECSGameLoopContainer> m_gameLoops;
};
