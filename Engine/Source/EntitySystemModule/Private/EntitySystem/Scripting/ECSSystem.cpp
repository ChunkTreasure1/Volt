#include "espch.h"

#include "EntitySystem/Scripting/ECSSystem.h"

ECSSystem::ECSSystem(UUID64 id, ECSSystemFunc&& func)
	: m_systemFunc(std::move(func)), m_id(id)
{
}

ECSSystem::~ECSSystem()
{
}

void ECSSystem::Execute(Volt::EntityScene& scene, float deltaTime)
{
	m_systemFunc(scene, deltaTime);
}

void ECSExecutionOrder::ExecuteAfter(ECSSystem& otherSystem)
{
	const auto id = otherSystem.GetID();
	VT_ENSURE(std::find(m_executeAfter.begin(), m_executeAfter.end(), id) == m_executeAfter.end());
	VT_ENSURE(std::find(m_executeBefore.begin(), m_executeBefore.end(), id) == m_executeBefore.end());

	m_executeAfter.emplace_back(id);
}

void ECSExecutionOrder::ExecuteBefore(ECSSystem& otherSystem)
{
	const auto id = otherSystem.GetID();
	VT_ENSURE(std::find(m_executeBefore.begin(), m_executeBefore.end(), id) == m_executeBefore.end());
	VT_ENSURE(std::find(m_executeAfter.begin(), m_executeAfter.end(), id) == m_executeAfter.end());

	m_executeBefore.emplace_back(id);
}
