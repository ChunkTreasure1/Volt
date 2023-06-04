#pragma once

#include <Wire/Entity.h>

#include <unordered_map>

namespace Volt
{
	class Scene;
}

class SelectionManager
{
public:
	static void Init();
	static void SetSelectionKey(const std::string& key);
	static void ResetSelectionKey();

	static bool Select(Wire::EntityId entity);
	static bool Deselect(Wire::EntityId entity);

	static void DeselectAll();
	static bool IsAnySelected();
	static bool IsSelected(Wire::EntityId entity);

	static void Update(Ref<Volt::Scene> scene);

	static bool IsAnyParentSelected(Wire::EntityId entity, Ref<Volt::Scene> scene);

	inline static int32_t& GetFirstSelectedRow() { return myFirstSelectedRow; }
	inline static int32_t& GetLastSelectedRow() { return myLastSelectedRow; }

	inline static const size_t GetSelectedCount() { return myEntities["Default"].size(); }
	inline static const std::vector<Wire::EntityId>& GetSelectedEntities() { return myEntities.at(mySelectionKey); }

	inline static void Lock() { myLocked = true; }
	inline static void Unlock() { myLocked = false; }

private:
	SelectionManager() = delete;

	inline static std::string mySelectionKey = "Default";
	inline static bool myLocked = false;

	inline static int32_t myFirstSelectedRow = -1;
	inline static int32_t myLastSelectedRow = -1;
	inline static std::unordered_map<std::string, std::vector<Wire::EntityId>> myEntities;
};
