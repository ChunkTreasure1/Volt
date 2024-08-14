#pragma once

#include "Sandbox/Window/EditorWindow.h"

#include <Volt/Events/ApplicationEvents.h>
#include <Volt/Scene/Entity.h>

namespace Volt
{
	class Material;
	class Mesh;
	class Camera;
	class Scene;
	class SceneRenderer;
	class WindowRenderEvent;
}

class MaterialEditorPanel : public EditorWindow
{
public:
	MaterialEditorPanel(Ref<Volt::Scene>& aScene);

	void UpdateMainContent() override;
	void UpdateContent() override;

	void OpenAsset(Ref<Volt::Asset> asset) override;
	void OnEvent(Volt::Event& e) override;

	void OnOpen() override;
	void OnClose() override;

private:
	bool OnRenderEvent(Volt::WindowRenderEvent& e);
	void UpdateToolbar();
	void UpdateProperties();
	void UpdatePreview();
	void UpdateSubMaterials();
	void UpdateMaterials();

	const float myButtonSize = 22.f;

	Ref<Volt::Camera> myPreviewCamera;

	Ref<Volt::Material> mySelectedMaterial;
	Ref<Volt::Scene>& myEditorScene;

	Ref<Volt::Scene> myPreviewScene;
	Ref<Volt::SceneRenderer> myPreviewRenderer;

	Volt::Entity myPreviewEntity;
	std::string mySearchQuery;
	bool myHasSearchQuery = false;
};
