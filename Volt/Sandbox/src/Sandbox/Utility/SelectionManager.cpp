#include "sbpch.h"
#include "SelectionManager.h"

#include <Volt/Core/Profiling.h>
#include <Volt/Scene/Entity.h>

void SelectionManager::Initialize()
{
	m_entities[SelectionContext::Scene] = std::vector<Volt::EntityID>();
}

bool SelectionManager::Select(Volt::EntityID entity, SelectionContext context)
{
	if (m_locked)
	{
		return false;
	}

	if (std::find(m_entities.at(context).begin(), m_entities.at(context).end(), entity) != m_entities.at(context).end())
	{
		return false;
	}

	m_entities.at(context).emplace_back(entity);
	return true;
}

bool SelectionManager::Deselect(Volt::EntityID entity, SelectionContext context)
{
	if (m_locked)
	{
		return false;
	}

	auto it = std::find(m_entities.at(context).begin(), m_entities.at(context).end(), entity);
	if (it == m_entities.at(context).end())
	{
		return false;
	}

	m_entities.at(context).erase(it);
	return true;
}

void SelectionManager::DeselectAll(SelectionContext context)
{
	if (m_locked)
	{
		return;
	}

	m_firstSelectedRow = -1;
	m_lastSelectedRow = -1;
	m_entities.at(context).clear();
}

bool SelectionManager::IsAnySelected(SelectionContext context)
{
	return !m_entities.at(context).empty();
}

bool SelectionManager::IsSelected(Volt::EntityID entity, SelectionContext context)
{
	return std::find(m_entities.at(context).begin(), m_entities.at(context).end(), entity) != m_entities.at(context).end();
}

void SelectionManager::Update(Ref<Volt::Scene> scene)
{
	VT_PROFILE_FUNCTION();

	for (const auto& ent : GetSelectedEntities())
	{
		if (!scene->IsEntityValid(ent))
		{
			SelectionManager::Deselect(ent);
		}
	}
}

bool SelectionManager::IsAnyParentSelected(Volt::EntityID id, Ref<Volt::Scene> scene)
{
	Volt::Entity entity = scene->GetEntityFromUUID(id);

	if (entity.GetParent())
	{
		if (IsSelected(entity.GetParent().GetID()))
		{
			return true;
		}

		return IsAnyParentSelected(entity.GetParent().GetID(), scene);
	}

	return false;
}
