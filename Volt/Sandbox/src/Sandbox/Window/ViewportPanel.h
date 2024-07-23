#pragma once

#include "Sandbox/Window/EditorWindow.h"
#include "Sandbox/Window/AssetBrowser/AssetCommon.h"
#include "Sandbox/Utility/EditorUtilities.h"
#include "Sandbox/Utility/AnimatedIcon.h"

#include "Sandbox/Sandbox.h"

#include <Volt/Events/MouseEvent.h>

#include <glm/glm.hpp>

#include <imgui.h>
#include <ImGuizmo.h>

struct GizmoEvent
{
	//[0] = transform
	//[1] = rotation
	//[2] = scale
	entt::entity myEntityId;
	std::array<glm::vec3, 3> myValue;
};

namespace Volt
{
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

	glm::vec2 GetSize() { return m_viewportSize; }
	glm::vec2 GetViewportLocalPosition(const ImVec2& mousePos);
	glm::vec2 GetViewportLocalPosition(const glm::vec2& mousePos);

private:
	bool OnMousePressed(Volt::MouseButtonPressedEvent& e);
	bool OnKeyPressedEvent(Volt::KeyPressedEvent& e);
	bool OnMouseReleased(Volt::MouseButtonReleasedEvent& e);

	void CheckDragDrop();
	void UpdateCreatedEntityPosition();

	void DuplicateSelection();
	void HandleSingleSelect();
	void HandleMultiSelect();
	void HandleSingleGizmoInteraction(const glm::mat4& avgTransform);
	void HandleMultiGizmoInteraction(const glm::mat4& deltaTransform);

	void UpdateModals();
	void HandleNonMeshDragDrop();

	void Resize(const glm::vec2& viewportSize);

	glm::mat4 CalculateAverageTransform();

	Ref<Volt::SceneRenderer>& m_sceneRenderer;
	Ref<Volt::Scene>& m_editorScene;

	AnimatedIcon m_animatedPhysicsIcon;
	EditorCameraController* m_editorCameraController;

	glm::vec2 m_perspectiveBounds[2] = { { 0.f, 0.f }, { 0.f, 0.f } };
	glm::vec2 m_viewportSize = { 1280.f, 720.f };

	ImGuizmo::OPERATION m_gizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
	SceneState& m_sceneState;

	const Vector<float> m_snapToGridValues = { 1.f, 10.f, 25.f, 50.f, 100.f, 200.f, 500.f, 1000.f };
	const Vector<float> m_snapRotationValues = { 10.f, 30.f, 45.f, 90.f };
	const Vector<float> m_snapScaleValues = { 0.01f, 0.1f, 0.25f, 0.5f, 1.f };

	glm::vec2 m_viewportMouseCoords;
	bool m_midEvent;

	bool m_createdAssetOnDrag = false;
	bool m_isInViewport = false;

	bool m_isDragging = false;
	bool m_beganClick = false;
	ImVec2 m_startDragPos = { 0.f, 0.f };

	uint32_t m_currentRenderPass = 0;

	Volt::Entity m_createdEntity;

	Volt::EntityID m_entityToAddMesh = Volt::Entity::NullID();
	std::filesystem::path m_meshToImport;

	Volt::AssetHandle m_sceneToOpen = Volt::Asset::Null();

	///// Modals /////
	UUID64 m_meshImportModal = 0;
};

