#pragma once

#include "Sandbox/Window/EditorWindow.h"
#include "Sandbox/Window/AssetBrowser/AssetCommon.h"
#include "Sandbox/Utility/EditorUtilities.h"
#include "Sandbox/Utility/AnimatedIcon.h"

#include "Sandbox/Sandbox.h"

#include <Volt/Events/MouseEvent.h>

#include <glm/glm.hpp>
#include <Wire/Entity.h>

#include <imgui.h>
#include <ImGuizmo.h>

namespace Volt
{
	class SceneRenderer;
	class Scene;
}

class EditorCameraController;
class GameViewPanel : public EditorWindow
{
public:
	GameViewPanel(Ref<Volt::SceneRenderer>& sceneRenderer, Ref<Volt::Scene>& editorScene, SceneState& aSceneState);

	void UpdateMainContent() override;
	void OnEvent(Volt::Event& e) override;

	void OnOpen() override;
	void OnClose() override;

	glm::vec2 GetViewportLocalPosition(const glm::vec2& mousePos);

	inline static const std::string GAMEVIEWPANEL_TITLE = "Game Viewport";

private:
	bool OnMouseMoved(Volt::MouseMovedEvent& e);
	bool OnMousePressed(Volt::MouseButtonPressedEvent& e);
	bool OnKeyPressedEvent(Volt::KeyPressedEvent& e);
	bool OnMouseReleased(Volt::MouseButtonReleasedEvent& e);

	void Resize(const glm::vec2& viewportSize);

	Ref<Volt::SceneRenderer>& mySceneRenderer;
	Ref<Volt::Scene>& myEditorScene;
	SceneState& mySceneState;

	glm::vec2 myPerspectiveBounds[2] = { { 0.f, 0.f }, { 0.f, 0.f } };
	glm::vec2 myViewportSize = { 1280.f, 720.f };
};

