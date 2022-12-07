#pragma once

#include <Wire/Entity.h>

class SelectionManager
{
public:
	static bool Select(Wire::EntityId entity);
	static bool Deselect(Wire::EntityId entity);

	static void DeselectAll();
	static bool IsAnySelected();
	static bool IsSelected(Wire::EntityId entity);

	inline static const size_t GetSelectedCount() { return myEntities.size(); }
	inline static const std::vector<Wire::EntityId>& GetSelectedEntities() { return myEntities; }

private:
	SelectionManager() = delete;

	inline static std::vector<Wire::EntityId> myEntities;
};