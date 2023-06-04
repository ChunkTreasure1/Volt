#pragma once

#include "Sandbox/Window/EditorWindow.h"
#include "Sandbox/Utility/Helpers.h"

#include <Volt/Scene/Entity.h>
#include <Volt/Scene/Scene.h>

#include <Volt/Events/KeyEvent.h>

#include <vector>

namespace Volt
{
	class Prefab;
}

class SceneViewPanel : public EditorWindow
{
public:
	SceneViewPanel(Ref<Volt::Scene>& scene, const std::string& id);
	void UpdateMainContent() override;
	void OnEvent(Volt::Event& e) override;

	void HighlightEntity(Wire::EntityId id);

private:
	bool OnKeyPressedEvent(Volt::KeyPressedEvent& e);

	void DrawEntity(Wire::EntityId id, const std::string& filter);
	void CreatePrefabAndSetupEntities(Wire::EntityId entity);

	void RebuildEntityDrawList();
	void RebuildEntityDrawListRecursive(Wire::EntityId entityId, const std::string& filter);

	bool SearchRecursively(Wire::EntityId id, const std::string& filter, uint32_t maxSearchDepth, uint32_t currentDepth = 0);
	bool SearchRecursivelyParent(Wire::EntityId id, const std::string& filter, uint32_t maxSearchDepth, uint32_t currentDepth = 0);
	bool MatchesQuery(const std::string& text, const std::string& filter);
	bool HasComponent(Wire::EntityId id, const std::string& filter);
	bool HasScript(Wire::EntityId id, const std::string& filter);

	void DrawMainRightClickPopup();

	void CorrectMissingPrefabs();
	void ReloadAllPrefabModal();
	void ReloadPrefabImpl(Wire::EntityId id, Ref<Volt::Prefab> asset);

	std::string mySearchQuery;
	bool myHasSearchQuery = false;
	Wire::EntityId myScrollToEntiy = 0;

	bool myIsRenamingLayer = false;
	uint32_t myRenamingLayer = 0;

	std::vector<Wire::EntityId> myEntityDrawList;
	std::unordered_map<Wire::EntityId, ImGuiID> myEntityToImGuiID;
	bool myRebuildDrawList = false;

	Ref<Volt::Scene>& myScene;
};
