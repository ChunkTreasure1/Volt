#pragma once
#pragma once

#include "Sandbox/Window/EditorWindow.h"
#include "Sandbox/Window/AssetCommon.h"
#include "Sandbox/Utility/EditorUtilities.h"
#include "Sandbox/Utility/AnimatedIcon.h"

#include "Sandbox/Sandbox.h"

#include <Volt/Events/MouseEvent.h>

#include <gem/gem.h>
#include <Wire/Entity.h>

#include <imgui.h>
#include <ImGuizmo.h>

struct GizmoEvent
{
	//[0] = transform
	//[1] = rotation
	//[2] = scale
	Wire::EntityId myEntityId;
	std::array<gem::vec3, 3> myValue;
};

namespace Volt
{
	class Framebuffer;
	class SceneRenderer;
	class Scene;
}

class EditorCameraController;
class ViewportPanel : public EditorWindow
{
public:
	ViewportPanel(Ref<Volt::SceneRenderer>& sceneRenderer, Ref<Volt::Scene>& editorScene, EditorCameraController* cameraController, SceneState& aSceneState);

	void UpdateMainContent() override;
	void UpdateContent() override;
	void OnEvent(Volt::Event& e) override;

private:
	bool OnMouseMoved(Volt::MouseMovedEvent& e);
	bool OnMousePressed(Volt::MouseButtonPressedEvent& e);
	bool OnKeyPressedEvent(Volt::KeyPressedEvent& e);
	bool OnMouseReleased(Volt::MouseButtonReleasedEvent& e);

	void CheckDragDrop();
	void UpdateCreatedEntityPosition();

	void DuplicateSelection();
	void HandleSingleSelect();
	void HandleMultiSelect();
	void HandleSingleGizmoInteraction(const gem::mat4& avgTransform);
	void HandleMultiGizmoInteraction(const gem::mat4& avgTransform, const gem::mat4& avgStartTransform);

	void UpdateModals();
	void HandleNonMeshDragDrop();

	void Resize(const gem::vec2& viewportSize);

	gem::vec2 GetViewportLocalPosition(const ImVec2& mousePos);
	gem::mat4 CalculateAverageTransform();

	Ref<Volt::SceneRenderer>& mySceneRenderer;
	Ref<Volt::Scene>& myEditorScene;

	AnimatedIcon myAnimatedPhysicsIcon;
	EditorCameraController* myEditorCameraController;

	gem::vec2 myPerspectiveBounds[2] = { { 0.f, 0.f }, { 0.f, 0.f } };
	gem::vec2 myViewportSize = { 1280.f, 720.f };

	ImGuizmo::OPERATION myGizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
	SceneState& mySceneState;

	const std::vector<float> m_snapToGridValues = { 1.f, 10.f, 25.f, 50.f, 100.f, 200.f, 500.f, 1000.f };
	const std::vector<float> m_snapRotationValues = { 10.f, 30.f, 45.f, 90.f };
	const std::vector<float> m_snapScaleValues = { 0.01f, 0.1f, 0.25f, 0.5f, 1.f };

	gem::vec2 myViewportMouseCoords;
	bool myMidEvent;

	bool myCreatedAssetOnDrag = false;
	bool myIsInViewport = false;
	
	bool myShowRenderTargets = false;

	bool myIsDragging = false;
	bool myBeganClick = false;
	ImVec2 myStartDragPos = { 0.f, 0.f };

	uint32_t myCurrentRenderPass = 0;

	Volt::Entity myCreatedEntity;

	Wire::EntityId myEntityToAddMesh = Wire::NullID;
	std::filesystem::path myMeshToImport;
	MeshImportData myMeshImportData;

	Volt::AssetHandle mySceneToOpen = Volt::Asset::Null();
};

