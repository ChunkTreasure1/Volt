#include "sbpch.h"
#include "SelectionManager.h"

#include <Volt/Core/Profiling.h>
#include <Volt/Scene/Entity.h>

void SelectionManager::Init()
{
	myEntities[mySelectionKey] = std::vector<entt::entity>();
}

void SelectionManager::SetSelectionKey(const std::string& key)
{
	mySelectionKey = key;
	if (!myEntities.contains(mySelectionKey))
	{
		myEntities[mySelectionKey] = std::vector<entt::entity>();
	}
}

void SelectionManager::ResetSelectionKey()
{
	mySelectionKey = "";
}

bool SelectionManager::Select(entt::entity entity)
{
	if (myLocked) return false;

	if (std::find(myEntities.at(mySelectionKey).begin(), myEntities.at(mySelectionKey).end(), entity) != myEntities.at(mySelectionKey).end())
	{
		return false;
	}

	myEntities.at(mySelectionKey).emplace_back(entity);
	return true;
}

bool SelectionManager::Deselect(entt::entity entity)
{
	if (myLocked) return false;

	auto it = std::find(myEntities.at(mySelectionKey).begin(), myEntities.at(mySelectionKey).end(), entity);
	if (it == myEntities.at(mySelectionKey).end())
	{
		return false;
	}

	myEntities.at(mySelectionKey).erase(it);
	return true;
}

void SelectionManager::DeselectAll()
{
	if (myLocked) return;

	myFirstSelectedRow = -1;
	myLastSelectedRow = -1;
	myEntities.at(mySelectionKey).clear();
}

bool SelectionManager::IsAnySelected()
{
	return !myEntities.at(mySelectionKey).empty();
}

bool SelectionManager::IsSelected(entt::entity entity)
{
	return std::find(myEntities.at(mySelectionKey).begin(), myEntities.at(mySelectionKey).end(), entity) != myEntities.at(mySelectionKey).end();
}

void SelectionManager::Update(Ref<Volt::Scene> scene)
{
	VT_PROFILE_FUNCTION();

	for (const auto& ent : GetSelectedEntities())
	{
		if (!scene->GetRegistry().valid(ent))
		{
			SelectionManager::Deselect(ent);
		}
	}
}

bool SelectionManager::IsAnyParentSelected(entt::entity id, Ref<Volt::Scene> scene)
{
	Volt::Entity entity{ id, scene.get() };

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
