#pragma once

#include <Volt/Scene/EntityID.h>

#include <unordered_map>

namespace Volt
{
	class Scene;
}

enum class SelectionContext
{
	Scene,

	GameUIEditor
};

class SelectionManager
{
public:
	static void Initialize();

	static bool Select(Volt::EntityID entity, SelectionContext context = SelectionContext::Scene);
	static bool Deselect(Volt::EntityID entity, SelectionContext context = SelectionContext::Scene);

	static void DeselectAll(SelectionContext context = SelectionContext::Scene);
	static bool IsAnySelected(SelectionContext context = SelectionContext::Scene);
	static bool IsSelected(Volt::EntityID entity, SelectionContext context = SelectionContext::Scene);

	static void Update(Ref<Volt::Scene> scene);

	static bool IsAnyParentSelected(Volt::EntityID entity, Ref<Volt::Scene> scene);

	inline static int32_t& GetFirstSelectedRow() { return m_firstSelectedRow; }
	inline static int32_t& GetLastSelectedRow() { return m_lastSelectedRow; }

	inline static const size_t GetSelectedCount(SelectionContext context = SelectionContext::Scene) { return m_entities[context].size(); }
	inline static const Vector<Volt::EntityID>& GetSelectedEntities(SelectionContext context = SelectionContext::Scene) { return m_entities[context]; }

	inline static void Lock() { m_locked = true; }
	inline static void Unlock() { m_locked = false; }

private:
	SelectionManager() = delete;

	inline static int32_t m_firstSelectedRow = -1;
	inline static int32_t m_lastSelectedRow = -1;
	inline static bool m_locked = false;
	inline static std::unordered_map<SelectionContext, Vector<Volt::EntityID>> m_entities;
};
