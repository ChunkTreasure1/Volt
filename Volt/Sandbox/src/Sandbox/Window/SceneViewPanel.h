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
	void DrawEntity(Wire::EntityId id, const std::string& filter);
	void CreatePrefabAndSetupEntities(Wire::EntityId entity);
	void SetupEntityAsPrefab(Wire::EntityId entity, Volt::AssetHandle prefabId);

	bool SearchRecursivly(Wire::EntityId id, const std::string& filter, uint32_t maxSearchDepth, uint32_t currentDepth = 0);
	bool MatchesQuery(const std::string& text, const std::string& filter);

	std::string mySearchQuery;
	bool myHasSearchQuery = false;

	Ref<Volt::Scene>& myScene;
};