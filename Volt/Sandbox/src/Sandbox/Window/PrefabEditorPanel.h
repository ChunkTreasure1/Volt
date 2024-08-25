#pragma once

#include "Sandbox/Window/EditorWindow.h"

#include "Sandbox/Window/SceneViewPanel.h"
#include "Sandbox/Window/PropertiesPanel.h"

#include <Volt/Public/Events/ApplicationEvents.h>
#include <Volt/Scene/Entity.h>

namespace Volt
{
	class SceneRenderer;
	class Scene;
	class Mesh;
	class Material;
}

class EditorCameraController;
class PrefabEditorPanel : public EditorWindow
{
public:
	PrefabEditorPanel();
	~PrefabEditorPanel() override = default;

	void UpdateMainContent() override;
	void OpenAsset(Ref<Volt::Asset> asset) override;

	void OnOpen() override;
	void OnClose() override;

private:
	bool OnRenderEvent(Volt::WindowRenderEvent& e);

	void UpdateViewport();
	void UpdateSceneView();
	void UpdateProperties();
	void UpdateToolbar();
	void UpdateMeshList();

	void SaveCurrentMesh();

	Ref<Volt::Scene> myScene;
	Ref<Volt::SceneRenderer> mySceneRenderer;

	glm::vec2 myPerspectiveBounds[2] = { { 0.f, 0.f }, { 0.f, 0.f } };
	glm::vec2 myViewportSize = { 1280.f, 720.f };

	Volt::Entity myPreviewEntity;
	Ref<Volt::Mesh> myCurrentMesh;
	Ref<Volt::Mesh> myCurrentPrefab;

	const float myButtonSize = 22.f;
	uint32_t mySelectedSubMesh = 0;

	Ref<EditorCameraController> myCameraController;

	SceneState mySceneState;

	Ref<SceneViewPanel> mySceneViewPanel;
	Ref<PropertiesPanel> myPropertiesPanel;
};
