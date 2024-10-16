#include "sspch.h"

#include "SubSystem/SubSystemManager.h"
#include "SubSystem/SubSystem.h"

SubSystemManager::SubSystemManager()
{
	VT_ASSERT(!s_instance);
	s_instance = this;
}

SubSystemManager::~SubSystemManager()
{
	s_instance = nullptr;
}

void SubSystemManager::InitializeSubSystems(SubSystemInitializationStage initializationStage)
{
	struct SortedSubSystem
	{
		VoltGUID guid;
		int32_t initializationOrder;
	};

	const auto& registeredSubSystems = GetSubSystemRegistry().GetRegisteredSubSystems();

	Vector<SortedSubSystem> sortedSubSystems;

	for (const auto& [guid, registeredSubSystem] : registeredSubSystems)
	{
		if (registeredSubSystem.initializationStage != initializationStage)
		{
			continue;
		}

		auto& sortedSubSystem = sortedSubSystems.emplace_back();
		sortedSubSystem.guid = guid;
		sortedSubSystem.initializationOrder = registeredSubSystem.initializationOrder;
	}

	// There are no sub systems at this stage.
	if (sortedSubSystems.empty())
	{
		return;
	}

	std::sort(sortedSubSystems.begin(), sortedSubSystems.end(), [](const SortedSubSystem& lhs, const SortedSubSystem& rhs)
	{
		return lhs.initializationOrder < rhs.initializationOrder;
	});

	for (const auto& sortedSubSystem : sortedSubSystems)
	{
		m_subSystems[initializationStage].emplace_back(registeredSubSystems.at(sortedSubSystem.guid).factoryFunction());
		m_subSystemsMap[sortedSubSystem.guid] = m_subSystems[initializationStage].back();
	}

	for (auto subSystem : m_subSystems[initializationStage])
	{
		subSystem->Initialize();
	}
}

void SubSystemManager::ShutdownSubSystems(SubSystemInitializationStage initializationStage)
{
	// Ensure correct destructor order (event listeners may depend on this)
	m_subSystemsMap.clear();

	for (auto& subSystem : std::ranges::reverse_view(m_subSystems[initializationStage]))
	{
		subSystem->Shutdown();
	}

	for (auto& subSystem : std::ranges::reverse_view(m_subSystems[initializationStage]))
	{
		subSystem = nullptr;
	}
}
