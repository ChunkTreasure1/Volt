#pragma once

#include "Sandbox/Window/EditorWindow.h"
#include "Sandbox/Window/AssetCommon.h"

#include <Volt/Scene/Entity.h>
#include <Volt/Scene/Scene.h>

#include <vector>

class CreatePanel : public EditorWindow
{
public:
	CreatePanel(std::vector<Wire::EntityId>& selectedEntites, Ref<Volt::Scene>& scene);
	void UpdateMainContent() override;

private:
	std::shared_ptr<DirectoryData> ProcessDirectory(const std::filesystem::path& path, std::shared_ptr<DirectoryData> parent);
	void RenderDirectory(const std::shared_ptr<DirectoryData> dirData, uint32_t& i);

	std::vector<Wire::EntityId>& mySelectedEntites;

	Ref<Volt::Scene>& myScene;

	std::unordered_map<std::string, Ref<DirectoryData>> myDirectories;
	DirectoryData* myCurrentDirectory = nullptr;

	bool myMeshListOpen = false;
	bool myHasSearchQuery = false;
	const float myButtonSize = 24.f;

	std::string mySearchQuery;
};