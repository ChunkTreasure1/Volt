#pragma once

#include "Sandbox/Window/EditorWindow.h"

#include <Volt/Events/ApplicationEvent.h>
#include <Volt/Scene/Entity.h>

namespace Volt 
{
	class SceneRenderer;
	class Scene;
	class Mesh;
	class Material;
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

	void OnEvent(Volt::Event& e) override;

private:
	bool OnRenderEvent(Volt::AppRenderEvent& e);

	void UpdateViewport();
	void UpdateProperties();
	void UpdateToolbar();
	void UpdateMeshList();

	void SaveCurrentMesh();
	void FlipV();

	Ref<Volt::Scene> myScene;
	Ref<Volt::SceneRenderer> mySceneRenderer;
	Ref<Volt::Material> myGridMaterial;

	gem::vec2 myPerspectiveBounds[2] = { { 0.f, 0.f }, { 0.f, 0.f } };
	gem::vec2 myViewportSize = { 1280.f, 720.f };

	Volt::Entity myPreviewEntity;
	Ref<Volt::Mesh> myCurrentMesh;

	const float myButtonSize = 22.f;
	uint32_t mySelectedSubMesh = 0;

	Ref<EditorCameraController> myCameraController;
};

