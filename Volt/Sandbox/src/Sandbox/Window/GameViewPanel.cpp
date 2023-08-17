#include "sbpch.h"
#include "GameViewPanel.h"

#include "Sandbox/Camera/EditorCameraController.h"
#include "Sandbox/Utility/SelectionManager.h"
#include "Sandbox/Utility/GlobalEditorStates.h"
#include "Sandbox/Utility/EditorResources.h"
#include "Sandbox/UserSettingsManager.h"
#include "Sandbox/Sandbox.h"

#include <Volt/Asset/Mesh/Mesh.h>
#include <Volt/Asset/Mesh/Material.h>
#include <Volt/Asset/ParticlePreset.h>
#include <Volt/Asset/Prefab.h>
#include <Volt/Components/Components.h>

#include <Volt/Input/Input.h>
#include <Volt/Input/KeyCodes.h>
#include <Volt/Input/MouseButtonCodes.h>

#include <Volt/RenderingNew/SceneRendererNew.h>
#include <Volt/Rendering/VulkanFramebuffer.h>
#include <Volt/Rendering/Camera/Camera.h>

#include <Volt/Scene/Entity.h>
#include <Volt/Utility/UIUtility.h>

#include <Volt/Utility/StringUtility.h>

#include "Sandbox/EditorCommandStack.h"

GameViewPanel::GameViewPanel(Ref<Volt::SceneRendererNew>& sceneRenderer, Ref<Volt::Scene>& editorScene, SceneState& aSceneState)
	: EditorWindow(GAMEVIEWPANEL_TITLE), mySceneRenderer(sceneRenderer), myEditorScene(editorScene),
	mySceneState(aSceneState)
{
	myIsOpen = true;
	myWindowFlags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
}

void GameViewPanel::UpdateMainContent()
{
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 0.f, 0.f });
	ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2{ 0.f, 0.f });
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0.f, 0.f });
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0.f, 0.f });
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4{ 0.07f, 0.07f, 0.07f, 1.f });

	auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
	auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
	auto viewportOffset = ImGui::GetWindowPos();

	myPerspectiveBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
	myPerspectiveBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

	ImVec2 viewportSize = ImGui::GetContentRegionAvail();

	if (myViewportSize != (*(glm::vec2*)&viewportSize) && viewportSize.x > 0 && viewportSize.y > 0)
	{
		Resize({ viewportSize.x, viewportSize.y });
	}

	if (mySceneRenderer)
	{
		ImGui::Image(UI::GetTextureID(mySceneRenderer->GetFinalImage()), { myViewportSize.x, myViewportSize.y });
	}

	ImGui::PopStyleColor();
	ImGui::PopStyleVar(4);
}

void GameViewPanel::OnEvent(Volt::Event& e)
{
	Volt::EventDispatcher dispatcher(e);
	dispatcher.Dispatch<Volt::MouseMovedEvent>(VT_BIND_EVENT_FN(GameViewPanel::OnMouseMoved));
	dispatcher.Dispatch<Volt::KeyPressedEvent>(VT_BIND_EVENT_FN(GameViewPanel::OnKeyPressedEvent));
	dispatcher.Dispatch<Volt::MouseButtonPressedEvent>(VT_BIND_EVENT_FN(GameViewPanel::OnMousePressed));
	dispatcher.Dispatch<Volt::MouseButtonReleasedEvent>(VT_BIND_EVENT_FN(GameViewPanel::OnMouseReleased));
}

void GameViewPanel::OnOpen()
{
	Volt::SceneRendererSpecification spec{};
	spec.debugName = "Game Viewport";
	spec.scene = myEditorScene;
	mySceneRenderer = CreateRef<Volt::SceneRendererNew>(spec);
}

void GameViewPanel::OnClose()
{
	mySceneRenderer = nullptr;
}

glm::vec2 GameViewPanel::GetViewportLocalPosition(const glm::vec2& mousePos)
{
	float mx = mousePos.x;
	float my = mousePos.y;
	mx -= myPerspectiveBounds[0].x;
	my -= myPerspectiveBounds[0].y;
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
		case VT_MOUSE_BUTTON_RIGHT:
			if (myIsHovered)
			{
				ImGui::SetWindowFocus("Game Viewport");
			}
			break;

		case VT_MOUSE_BUTTON_LEFT:
		{
			if (myIsHovered && mySceneState == SceneState::Play)
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
	if (!myIsHovered || Volt::Input::IsMouseButtonDown(VT_MOUSE_BUTTON_RIGHT) || ImGui::IsAnyItemActive())
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
	myViewportSize = { viewportSize.x, viewportSize.y };

	if (UserSettingsManager::GetSettings().sceneSettings.use16by9)
	{
		if (myViewportSize.x > myViewportSize.y)
		{
			myViewportSize.x = myViewportSize.y / 9 * 16;
		}
		else
		{
			myViewportSize.y = myViewportSize.x / 16 * 9;
		}
	}

	mySceneRenderer->Resize((uint32_t)myViewportSize.x, (uint32_t)myViewportSize.y);
	myEditorScene->SetRenderSize((uint32_t)myViewportSize.x, (uint32_t)myViewportSize.y);
}
