#pragma once

#include "Sandbox/Window/EditorWindow.h"

#include <Volt/Public/Events/ApplicationEvents.h>
#include <Volt/Rendering/UISceneRenderer.h>

#include <InputModule/Events/MouseEvents.h>
#include <WindowModule/Events/WindowEvents.h>

#include <RHIModule/Images/Image.h>
#include <CoreUtilities/Pointers/RefPtr.h>

namespace Volt
{
	class UIScene;

}

class GameUIEditorPanel : public EditorWindow
{
public:
	GameUIEditorPanel();
	~GameUIEditorPanel() override = default;

	void UpdateMainContent() override {}
	void UpdateContent() override;

	void OnOpen() override;
	void OnClose() override;

private:
	bool OnRenderEvent(Volt::WindowRenderEvent& e);
	bool OnMouseScrollEvent(Volt::MouseScrolledEvent& e);
	bool OnMouseButtonPressedEvent(Volt::MouseButtonPressedEvent& e);
	bool OnMouseButtonReleasedEvent(Volt::MouseButtonReleasedEvent& e);
	bool OnMouseMovedEvent(Volt::MouseMovedEvent& e);

	glm::vec2 GetViewportLocalPosition(const ImVec2& mousePos);

	void UpdateViewport();
	void UpdateHierarchy();
	void UpdateDetails();

	void HandleSelection();

	void CreateViewportImage(const uint32_t width, const uint32_t height);

	float m_currentZoom = 1.f;
	glm::vec2 m_currentPosition = 0.f;
	
	glm::vec2 m_previousMousePosition = 0.f;
	glm::vec2 m_currentMousePosition = 0.f;
	glm::vec2 m_viewportMouseCoords = 0.f;

	bool m_viewportHovered = false;
	bool m_middleMouseDown = false;

	glm::vec2 m_viewportBounds[2] = { { 0.f, 0.f }, { 0.f, 0.f } };
	glm::vec2 m_viewportSize = { 1280.f, 720.f };

	Ref<Volt::UIScene> m_uiScene;
	RefPtr<Volt::RHI::Image> m_viewportImage;

	Scope<Volt::UISceneRenderer> m_uiSceneRenderer;
};
