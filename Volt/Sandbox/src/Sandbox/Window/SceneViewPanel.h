#pragma once

#include "Sandbox/Window/EditorWindow.h"
#include "Sandbox/Utility/Helpers.h"

#include <Volt/Scene/Entity.h>
#include <Volt/Scene/Scene.h>

#include <vector>

class SceneViewPanel : public EditorWindow
{
public:
	SceneViewPanel(Ref<Volt::Scene>& scene);
	void UpdateMainContent() override;

private:
	void Search(const std::string query);

	void DrawEntity(Wire::EntityId id, const std::vector<Wire::EntityId>& usedEntities);
	void CreatePrefabAndSetupEntities(Wire::EntityId entity);
	void SetupEntityAsPrefab(Wire::EntityId entity, Volt::AssetHandle prefabId);

	void SelectEntitiesBetweenClosest(Wire::EntityId entity, const std::vector<Wire::EntityId>& usedEntities);
	void RecursivlySelectChildren(Wire::EntityId entity);

	std::string mySearchQuery;
	bool myHasSearchQuery = false;

	std::vector<Wire::EntityId> mySearchEntities;

	Ref<Volt::Scene>& myScene;
};