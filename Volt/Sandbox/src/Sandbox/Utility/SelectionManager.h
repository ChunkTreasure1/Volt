#pragma once

#include <Wire/Entity.h>

namespace Volt
{
	class Scene;
}

class SelectionManager
{
public:
	static bool Select(Wire::EntityId entity);
	static bool Deselect(Wire::EntityId entity);

	static void DeselectAll();
	static bool IsAnySelected();
	static bool IsSelected(Wire::EntityId entity);

	static bool IsAnyParentSelected(Wire::EntityId entity, Ref<Volt::Scene> scene);

	inline static int32_t& GetFirstSelectedRow() { return myFirstSelectedRow; }
	inline static int32_t& GetLastSelectedRow() { return myLastSelectedRow; }

	inline static const size_t GetSelectedCount() { return myEntities.size(); }
	inline static const std::vector<Wire::EntityId>& GetSelectedEntities() { return myEntities; }

private:
	SelectionManager() = delete;

	inline static int32_t myFirstSelectedRow = -1;
	inline static int32_t myLastSelectedRow = -1;
	inline static std::vector<Wire::EntityId> myEntities;
};