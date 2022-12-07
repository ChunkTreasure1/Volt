#include "sbpch.h"
#include "SelectionManager.h"

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