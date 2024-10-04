#include "espch.h"
#include "Scripting/ECSBuilder.h"

#include <queue>

ECSBuilder::ECSBuilder(ScriptingEngine& scriptingEngine)
	: m_scriptingEngine(scriptingEngine)
{
}

void ECSBuilder::Compile()
{
	for (auto& [type, loop] : m_gameLoops)
	{
		loop.Compile();
	}
}

void ECSGameLoopContainer::Compile()
{
	Vector<Vector<UUID64>> executionBuckets;
	vt::map<UUID64, size_t> systemToBucketIndex;

	vt::map<UUID64, Vector<UUID64>> graph;
	vt::map<UUID64, size_t> inDegree;

	// Initialize graph and in degree
	executionBuckets.push_back();
	for (auto& [id, system] : m_registeredSystems)
	{
		graph[id] = Vector<UUID64>{};
		inDegree[id] = 0;
		systemToBucketIndex[id] = 0;
	}

	// Build the graph
	for (auto& [id, system] : m_registeredSystems)
	{
		for (const auto& otherSystemId : system.Order().m_executeAfter)
		{
			graph[otherSystemId].emplace_back(id);
			inDegree[id]++;
		}

		for (const auto& otherSystemId : system.Order().m_executeBefore)
		{
			graph[id].emplace_back(otherSystemId);
			inDegree[otherSystemId]++;
		}
	}

	// Perform topological sort.
	std::queue<UUID64> zeroInDegreeQueue;
	for (auto& [id, degree] : inDegree)
	{
		if (degree == 0)
		{
			zeroInDegreeQueue.push(id);
		}
	}

	while (!zeroInDegreeQueue.empty())
	{
		UUID64 current = zeroInDegreeQueue.front();
		zeroInDegreeQueue.pop();

		size_t currentBucket = systemToBucketIndex[current];
		if (executionBuckets.size() <= currentBucket)
		{
			executionBuckets.emplace_back();
		}

		executionBuckets.at(currentBucket).emplace_back(current);

		for (const UUID64& neighbor : graph[current])
		{
			inDegree[neighbor]--;
			if (inDegree[neighbor] == 0)
			{
				zeroInDegreeQueue.push(neighbor);
				systemToBucketIndex[neighbor] = currentBucket + 1;
			}
		}
	}

#ifdef VT_DEBUG
	for (auto& [id, degree] : inDegree)
	{
		VT_ENSURE(degree == 0);
	}
#endif

	m_executionBuckets = executionBuckets;
}

ECSGameLoopContainer& ECSBuilder::GetGameLoop(GameLoop gameLoopType)
{
	return m_gameLoops[gameLoopType];
}

void ECSGameLoopContainer::Execute(Volt::EntityScene& scene, float deltaTime)
{
	VT_PROFILE_FUNCTION();

	for (const auto& ids : m_executionBuckets)
	{
		for (const auto& id : ids)
		{
			m_registeredSystems[id].Execute(scene, deltaTime);
		}
	}
}
