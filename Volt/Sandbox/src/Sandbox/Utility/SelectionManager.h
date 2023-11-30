#pragma once

#include <Volt/Scene/EntityID.h>

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

	static bool Select(Volt::EntityID entity);
	static bool Deselect(Volt::EntityID entity);

	static void DeselectAll();
	static bool IsAnySelected();
	static bool IsSelected(Volt::EntityID entity);

	static void Update(Ref<Volt::Scene> scene);

	static bool IsAnyParentSelected(Volt::EntityID entity, Ref<Volt::Scene> scene);

	inline static int32_t& GetFirstSelectedRow() { return myFirstSelectedRow; }
	inline static int32_t& GetLastSelectedRow() { return myLastSelectedRow; }

	inline static const size_t GetSelectedCount() { return myEntities["Default"].size(); }
	inline static const std::vector<Volt::EntityID>& GetSelectedEntities() { return myEntities.at(mySelectionKey); }

	inline static void Lock() { myLocked = true; }
	inline static void Unlock() { myLocked = false; }

private:
	SelectionManager() = delete;

	inline static std::string mySelectionKey = "Default";
	inline static bool myLocked = false;

	inline static int32_t myFirstSelectedRow = -1;
	inline static int32_t myLastSelectedRow = -1;
	inline static std::unordered_map<std::string, std::vector<Volt::EntityID>> myEntities;
};
