#include "sbpch.h"
#include "SelectionManager.h"

#include <Volt/Scene/Entity.h>

bool SelectionManager::Select(Wire::EntityId entity)
{
	if (std::find(myEntities.begin(), myEntities.end(), entity) != myEntities.end())
	{
		return false;
	}

	myEntities.emplace_back(entity);
	return true;
}

bool SelectionManager::Deselect(Wire::EntityId entity)
{
	auto it = std::find(myEntities.begin(), myEntities.end(), entity);
	if (it == myEntities.end())
	{
		return false;
	}

	myEntities.erase(it);
	return true;
}

void SelectionManager::DeselectAll()
{
	myFirstSelectedRow = -1;
	myLastSelectedRow = -1;
	myEntities.clear();
}

bool SelectionManager::IsAnySelected()
{
	return !myEntities.empty();
}

bool SelectionManager::IsSelected(Wire::EntityId entity)
{
	return std::find(myEntities.begin(), myEntities.end(), entity) != myEntities.end();
}

bool SelectionManager::IsAnyParentSelected(Wire::EntityId id, Ref<Volt::Scene> scene)
{
	Volt::Entity entity{ id, scene.get() };

	if (entity.GetParent())
	{
		if (IsSelected(entity.GetParent().GetId()))
		{
			return true;
		}

		return IsAnyParentSelected(entity.GetParent().GetId(), scene);
	}

	return false;
}
