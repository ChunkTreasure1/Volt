#include "sbpch.h"
#include "Window/GameViewPanel.h"

#include "Sandbox/Camera/EditorCameraController.h"
#include "Sandbox/Utility/SelectionManager.h"
#include "Sandbox/Utility/GlobalEditorStates.h"
#include "Sandbox/Utility/EditorResources.h"
#include "Sandbox/UserSettingsManager.h"
#include "Sandbox/Sandbox.h"

#include <Volt/Asset/Mesh/Mesh.h>
#include <Volt/Asset/ParticlePreset.h>
#include <Volt/Asset/Prefab.h>

#include <InputModule/Input.h>
#include <InputModule/InputCodes.h>
#include <InputModule/MouseButtonCodes.h>

#include <Volt/Rendering/SceneRenderer.h>
#include <Volt/Rendering/Camera/Camera.h>

#include <Volt/Scene/Entity.h>
#include <Volt/Utility/UIUtility.h>

#include <InputModule/Events/KeyboardEvents.h>

#include "Sandbox/EditorCommandStack.h"

GameViewPanel::GameViewPanel(Ref<Volt::SceneRenderer>& sceneRenderer, Ref<Volt::Scene>& editorScene, SceneState& aSceneState)
	: EditorWindow(GAMEVIEWPANEL_TITLE), m_sceneRenderer(sceneRenderer), m_editorScene(editorScene),
	m_sceneState(aSceneState)
{
	Open();
	m_windowFlags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
	m_isFullscreenImage = true;

	RegisterListener<Volt::MouseMovedEvent>(VT_BIND_EVENT_FN(GameViewPanel::OnMouseMoved));
	RegisterListener<Volt::KeyPressedEvent>(VT_BIND_EVENT_FN(GameViewPanel::OnKeyPressedEvent));
	RegisterListener<Volt::MouseButtonPressedEvent>(VT_BIND_EVENT_FN(GameViewPanel::OnMousePressed));
	RegisterListener<Volt::MouseButtonReleasedEvent>(VT_BIND_EVENT_FN(GameViewPanel::OnMouseReleased));
}

void GameViewPanel::UpdateMainContent()
{
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4{ 0.07f, 0.07f, 0.07f, 1.f });

	auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
	auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
	auto viewportOffset = ImGui::GetWindowPos();

	m_perspectiveBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
	m_perspectiveBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

	ImVec2 viewportSize = ImGui::GetContentRegionAvail();

	if (m_viewportSize != (*(glm::vec2*)&viewportSize) && viewportSize.x > 0 && viewportSize.y > 0)
	{
		Resize({ viewportSize.x, viewportSize.y });
	}

	if (m_sceneRenderer)
	{
		ImGui::Image(UI::GetTextureID(m_sceneRenderer->GetFinalImage()), { m_viewportSize.x, m_viewportSize.y });
	}

	ImGui::PopStyleColor();
}

void GameViewPanel::OnOpen()
{
	Volt::SceneRendererSpecification spec{};
	spec.debugName = "Game Viewport";
	spec.scene = m_editorScene;
	m_sceneRenderer = CreateRef<Volt::SceneRenderer>(spec);
}

void GameViewPanel::OnClose()
{
	m_sceneRenderer = nullptr;
}

glm::vec2 GameViewPanel::GetViewportLocalPosition(const glm::vec2& mousePos)
{
	float mx = mousePos.x;
	float my = mousePos.y;
	mx -= m_perspectiveBounds[0].x;
	my -= m_perspectiveBounds[0].y;
	glm::vec2 result = { mx, my };

	return result;
}

bool GameViewPanel::OnMouseMoved(Volt::MouseMovedEvent& e)
{
	return false;
}

bool GameViewPanel::OnMousePressed(Volt::MouseButtonPressedEvent& e)
{
	switch (e.GetMouseButton())
	{
		case Volt::InputCode::Mouse_RB:
			if (IsHovered())
			{
				ImGui::SetWindowFocus("Game Viewport");
			}
			break;

		case Volt::InputCode::Mouse_LB:
		{
			if (IsHovered() && m_sceneState == SceneState::Play)
			{
				Sandbox::Get().SetPlayHasMouseControl();
			}
			break;
		}
	}

	return false;
}

bool GameViewPanel::OnKeyPressedEvent(Volt::KeyPressedEvent& e)
{
	if (!IsHovered() || Volt::Input::IsMouseButtonDown(Volt::InputCode::Mouse_RB) || ImGui::IsAnyItemActive())
	{
		return false;
	}

	return false;
}

bool GameViewPanel::OnMouseReleased(Volt::MouseButtonReleasedEvent& e)
{
	return false;
}

void GameViewPanel::Resize(const glm::vec2& viewportSize)
{
	m_viewportSize = { viewportSize.x, viewportSize.y };

	if (UserSettingsManager::GetSettings().sceneSettings.use16by9)
	{
		if (m_viewportSize.x > m_viewportSize.y)
		{
			m_viewportSize.x = m_viewportSize.y / 9 * 16;
		}
		else
		{
			m_viewportSize.y = m_viewportSize.x / 16 * 9;
		}
	}

	m_sceneRenderer->Resize((uint32_t)m_viewportSize.x, (uint32_t)m_viewportSize.y);
	m_editorScene->SetRenderSize((uint32_t)m_viewportSize.x, (uint32_t)m_viewportSize.y);
}
