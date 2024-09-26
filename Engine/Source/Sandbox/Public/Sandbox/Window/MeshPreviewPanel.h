#pragma once

#include "Sandbox/Window/EditorWindow.h"

#include <Volt/Events/ApplicationEvents.h>
#include <Volt/Scene/Entity.h>

namespace Volt
{
	class SceneRenderer;
	class Scene;
	class Mesh;
	class Material;
	class WindowRenderEvent;
}

class EditorCameraController;
class MeshPreviewPanel : public EditorWindow
{
public:
	MeshPreviewPanel();
	~MeshPreviewPanel() override = default;

	void UpdateMainContent() override {}
	void UpdateContent() override;
	void OpenAsset(Ref<Volt::Asset> asset) override;

	void OnOpen() override;
	void OnClose() override;

private:
	bool OnRenderEvent(Volt::WindowRenderEvent& e);

	void UpdateViewport();
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

	const float myButtonSize = 22.f;
	int32_t mySelectedSubMesh = -1;

	Ref<EditorCameraController> myCameraController;
};

