#include "sbpch.h"
#include "SelectionManager.h"

#include <Volt/Core/Profiling.h>
#include <Volt/Scene/Entity.h>

void SelectionManager::Init()
{
	myEntities[mySelectionKey] = std::vector<Volt::EntityID>();
}

void SelectionManager::SetSelectionKey(const std::string& key)
{
	mySelectionKey = key;
	if (!myEntities.contains(mySelectionKey))
	{
		myEntities[mySelectionKey] = std::vector<Volt::EntityID>();
	}
}

void SelectionManager::ResetSelectionKey()
{
	mySelectionKey = "";
}

bool SelectionManager::Select(Volt::EntityID entity)
{
	if (myLocked) return false;

	if (std::find(myEntities.at(mySelectionKey).begin(), myEntities.at(mySelectionKey).end(), entity) != myEntities.at(mySelectionKey).end())
	{
		return false;
	}

	myEntities.at(mySelectionKey).emplace_back(entity);
	return true;
}

bool SelectionManager::Deselect(Volt::EntityID entity)
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

bool SelectionManager::IsSelected(Volt::EntityID entity)
{
	return std::find(myEntities.at(mySelectionKey).begin(), myEntities.at(mySelectionKey).end(), entity) != myEntities.at(mySelectionKey).end();
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
