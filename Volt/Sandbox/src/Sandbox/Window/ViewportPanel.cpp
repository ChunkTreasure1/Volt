#include "sbpch.h"
#include "ViewportPanel.h"

#include "Sandbox/Window/SceneViewPanel.h"
#include "Sandbox/Window/GameViewPanel.h"
#include "Sandbox/Camera/EditorCameraController.h"
#include "Sandbox/Utility/SelectionManager.h"
#include "Sandbox/Utility/GlobalEditorStates.h"
#include "Sandbox/Utility/EditorResources.h"
#include "Sandbox/Utility/EditorLibrary.h"
#include "Sandbox/UserSettingsManager.h"
#include "Sandbox/Sandbox.h"

#include "Sandbox/EditorCommandStack.h"

#include <Volt/Asset/Mesh/Mesh.h>
#include <Volt/Asset/Mesh/Material.h>
#include <Volt/Asset/ParticlePreset.h>
#include <Volt/Asset/Prefab.h>
#include <Volt/Components/Components.h>

#include <Volt/Input/Input.h>
#include <Volt/Input/KeyCodes.h>
#include <Volt/Input/MouseButtonCodes.h>

#include <Volt/Rendering/SceneRenderer.h>
#include <Volt/Rendering/VulkanFramebuffer.h>
#include <Volt/Rendering/Camera/Camera.h>
#include <Volt/Rendering/Texture/Image2D.h>

#include <Volt/Scene/Entity.h>
#include <Volt/Utility/UIUtility.h>

#include <Volt/Utility/StringUtility.h>
#include <Volt/Math/RayTriangle.h>
#include <Volt/Math/Math.h>

#include <Navigation/Core/NavigationSystem.h>

ViewportPanel::ViewportPanel(Ref<Volt::SceneRenderer>& sceneRenderer, Ref<Volt::Scene>& editorScene, EditorCameraController* cameraController,
	SceneState& aSceneState)
	: EditorWindow("Viewport"), mySceneRenderer(sceneRenderer), myEditorCameraController(cameraController), myEditorScene(editorScene),
	mySceneState(aSceneState), myAnimatedPhysicsIcon("Editor/Textures/Icons/Physics/LampPhysicsAnim1.dds", 30)
{
	myIsOpen = true;
	myWindowFlags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
	myMidEvent = false;
}

void ViewportPanel::UpdateMainContent()
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

	myViewportMouseCoords = GetViewportLocalPosition(ImGui::GetMousePos());

	ImVec2 viewportSize = ImGui::GetContentRegionAvail();

	if (myViewportSize != (*(glm::vec2*)&viewportSize) && viewportSize.x > 0 && viewportSize.y > 0)
	{
		Resize({ viewportSize.x, viewportSize.y });
	}

	auto& settings = UserSettingsManager::GetSettings();

	if (!myShowRenderTargets)
	{
		if (!settings.sceneSettings.use16by9)
		{
			ImGui::Image(UI::GetTextureID(mySceneRenderer->GetFinalImage()), { myViewportSize.x, myViewportSize.y });
		}
		else
		{
			ImGui::Image(UI::GetTextureID(mySceneRenderer->GetFinalImage()), { myViewportSize.x, myViewportSize.y });
		}
	}
	else
	{
		//const auto& passes = mySceneRenderer->GetAllFramebuffers();
		//const auto& [name, framebuffer] = passes.at(myCurrentRenderPass);

		//const uint32_t rowCount = (uint32_t)std::sqrt(framebuffer->GetSpecification().attachments.size()) + 1;

		//ImVec2 perViewportSize = { myViewportSize.x / rowCount, myViewportSize.y / rowCount };

		//ImGui::TextUnformatted(name.c_str());
		//for (uint32_t column = 0, i = 0; i < (uint32_t)framebuffer->GetSpecification().attachments.size(); i++)
		//{
		//	const auto& att = framebuffer->GetSpecification().attachments.at(i);
		//	if (Volt::Utility::IsDepthFormat(att.format) || Volt::Utility::IsTypeless(att.format))
		//	{
		//		continue;
		//	}

		//	const ImVec2 startPos = ImGui::GetCursorPos();
		//	
		//	// #TODO_Ivar: Reimplement

		//	//ImGui::Image(UI::GetTextureID(framebuffer->GetColorAttachment(i)), perViewportSize);

		//	column++;
		//	if (column < rowCount)
		//	{
		//		ImGui::SameLine();
		//	}
		//	else
		//	{
		//		column = 0;
		//	}
		//	const ImVec2 endPos = ImGui::GetCursorPos();

		//	ImGui::SetCursorPos(startPos);
		//	ImGui::TextUnformatted(framebuffer->GetColorAttachment(i)->GetSpecification().debugName.c_str());
		//	ImGui::SetCursorPos(endPos);
		//}
	}

	HandleNonMeshDragDrop();

	// Gizmo
	{
		static glm::mat4 averageTransform = glm::mat4(1.f);
		static glm::mat4 averageStartTransform = glm::mat4(1.f);

		static bool hasDuplicated = false;
		static bool isUsing = false;

		if (SelectionManager::IsAnySelected())
		{
			averageTransform = CalculateAverageTransform();
			bool snap = Volt::Input::IsKeyDown(VT_KEY_LEFT_CONTROL);
			const bool duplicate = Volt::Input::IsKeyDown(VT_KEY_LEFT_ALT);

			float snapValue = 0.5f;
			if (myGizmoOperation == ImGuizmo::ROTATE)
			{
				snap = settings.sceneSettings.snapRotation && snap ? false : settings.sceneSettings.snapRotation && !snap ? true : snap;
				snapValue = settings.sceneSettings.rotationSnapValue;
			}
			else if (myGizmoOperation == ImGuizmo::SCALE)
			{
				snap = settings.sceneSettings.snapScale && snap ? false : settings.sceneSettings.snapScale && !snap ? true : snap;
				snapValue = settings.sceneSettings.scaleSnapValue;
			}
			else if (myGizmoOperation == ImGuizmo::TRANSLATE)
			{
				snap = settings.sceneSettings.snapToGrid && snap ? false : settings.sceneSettings.snapToGrid && !snap ? true : snap;
				snapValue = settings.sceneSettings.gridSnapValue;
			}

			float snapValues[3] = { snapValue, snapValue, snapValue };

			ImGuizmo::MODE gizmoMode = settings.sceneSettings.worldSpace ? ImGuizmo::WORLD : ImGuizmo::LOCAL;

			ImGuizmo::SetOrthographic(false);
			ImGuizmo::SetDrawlist();

			ImGuizmo::SetRect(myPerspectiveBounds[0].x, myPerspectiveBounds[0].y, myPerspectiveBounds[1].x - myPerspectiveBounds[0].x, myPerspectiveBounds[1].y - myPerspectiveBounds[0].y);

			glm::mat4 deltaMatrix = { 1.f };

			// Temp camera is needed because 1/z messes up the gizmo while it's not in view
			const auto realCam = myEditorCameraController->GetCamera();
			Ref<Volt::Camera> tempCamera = CreateRef<Volt::Camera>(realCam->GetFieldOfView(), realCam->GetAspectRatio(), realCam->GetNearPlane(), realCam->GetFarPlane(), false);
			tempCamera->SetView(realCam->GetView());

			const auto view = tempCamera->GetView();
			const auto projection = tempCamera->GetProjection();

			ImGuizmo::Manipulate(
				glm::value_ptr(view),
				glm::value_ptr(projection),
				myGizmoOperation, gizmoMode, glm::value_ptr(averageTransform), glm::value_ptr(deltaMatrix), snap ? snapValues : nullptr);

			bool wasUsedPreviousFrame = isUsing;
			isUsing = ImGuizmo::IsUsing();

			if (wasUsedPreviousFrame && !isUsing)
			{
				for (auto ent : SelectionManager::GetSelectedEntities())
				{
					if (Sandbox::Get().CheckForUpdateNavMesh(Volt::Entity(ent, myEditorScene.get())))
					{
						Sandbox::Get().BakeNavMesh();
						break;
					}
				}
			}

			if (isUsing)
			{
				averageStartTransform = CalculateAverageTransform();

				if (duplicate && !hasDuplicated)
				{
					DuplicateSelection();
					hasDuplicated = true;
				}
				else if (averageTransform != averageStartTransform)
				{
					if (SelectionManager::GetSelectedCount() == 1)
					{
						HandleSingleGizmoInteraction(averageTransform);
					}
					else
					{
						HandleMultiGizmoInteraction(deltaMatrix);
					}
				}
			}
			else
			{
				hasDuplicated = false;
			}
		}

		myEditorCameraController->SetIsControllable(myIsHovered && !isUsing);
	}
	if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && ImGui::IsWindowHovered() && !ImGuizmo::IsOver() && !Volt::Input::IsKeyDown(VT_KEY_LEFT_ALT))
	{
		myBeganClick = true;
		HandleSingleSelect();
	}

	if (myBeganClick && ImGui::IsMouseDragging(ImGuiMouseButton_Left) && !Volt::Input::IsKeyDown(VT_KEY_LEFT_CONTROL) && !Volt::Input::IsKeyDown(VT_KEY_LEFT_SHIFT))
	{
		myStartDragPos = ImGui::GetMousePos();
		myIsDragging = true;
		myBeganClick = false;
	}

	if (myIsDragging && Volt::Input::IsKeyDown(VT_KEY_LEFT_ALT))
	{
		myIsDragging = false;
	}

	if (myIsDragging)
	{
		HandleMultiSelect();
	}

	if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
	{
		myIsDragging = false;
		myBeganClick = false;
	}

	ImGui::PopStyleColor();
	ImGui::PopStyleVar(4);

	UpdateModals();
	CheckDragDrop();
	UpdateCreatedEntityPosition();
}

void ViewportPanel::UpdateContent()
{
	if (myMidEvent && ImGui::IsMouseReleased(0))
	{
		myMidEvent = false;
	}

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 2.f));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0.f, 0.f));
	UI::ScopedColor button(ImGuiCol_Button, { 0.f, 0.f, 0.f, 0.f });
	UI::ScopedColor hovered(ImGuiCol_ButtonHovered, { 0.3f, 0.305f, 0.31f, 0.5f });
	UI::ScopedColor active(ImGuiCol_ButtonActive, { 0.5f, 0.505f, 0.51f, 0.5f });

	ImGui::Begin("##toolbar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoTabBar);

	const uint32_t rightButtonCount = 12;
	const float buttonSize = 22.f;

	auto& settings = UserSettingsManager::GetSettings();

	Ref<Volt::Texture2D> playIcon = EditorResources::GetEditorIcon(EditorIcon::Play);
	if (mySceneState == SceneState::Play)
	{
		playIcon = EditorResources::GetEditorIcon(EditorIcon::Stop);
	}

	if (UI::ImageButton("##play", UI::GetTextureID(playIcon), { buttonSize, buttonSize }))
	{
		static std::vector<Ref<EditorWindow>> fullscreenDeactivatedWindows;

		if (mySceneState == SceneState::Edit)
		{
			Sandbox::Get().OnScenePlay();

			if (settings.sceneSettings.fullscreenOnPlay)
			{
				for (const auto& window : EditorLibrary::GetPanels())
				{
					if (window.editorWindow->IsOpen() && window.editorWindow.get() != this && window.editorWindow.get()->GetTitle() != GameViewPanel::GAMEVIEWPANEL_TITLE && window.editorWindow->IsDocked())
					{
						const_cast<bool&>(window.editorWindow->IsOpen()) = false;
						fullscreenDeactivatedWindows.emplace_back(window.editorWindow);
					}
				}
			}
		}
		else if (mySceneState == SceneState::Play)
		{
			Sandbox::Get().OnSceneStop();

			if (settings.sceneSettings.fullscreenOnPlay)
			{
				for (const auto& window : fullscreenDeactivatedWindows)
				{
					const_cast<bool&>(window->IsOpen()) = true;
				}
				fullscreenDeactivatedWindows.clear();
			}
		}
	}

	ImGui::SameLine();

	Ref<Volt::Texture2D> physicsIcon = myAnimatedPhysicsIcon.GetCurrentFrame();
	static Ref<Volt::Texture2D> physicsId = physicsIcon;

	if (ImGui::ImageButtonAnimated(UI::GetTextureID(physicsId), UI::GetTextureID(physicsIcon), { buttonSize, buttonSize }))
	{
		if (mySceneState == SceneState::Edit)
		{
			Sandbox::Get().OnSimulationStart();
			myAnimatedPhysicsIcon.Play();
		}
		else if (mySceneState == SceneState::Simulating)
		{
			Sandbox::Get().OnSimulationStop();
			myAnimatedPhysicsIcon.Stop();
		}
	}

	ImGui::SameLine(ImGui::GetContentRegionAvail().x - (rightButtonCount * buttonSize));

	Ref<Volt::Texture2D> localWorldIcon;
	if (settings.sceneSettings.worldSpace)
	{
		localWorldIcon = EditorResources::GetEditorIcon(EditorIcon::WorldSpace);
	}
	else
	{
		localWorldIcon = EditorResources::GetEditorIcon(EditorIcon::LocalSpace);
	}

	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 2.f);

	if (UI::ImageButtonState("##fullscreenOnPlay", settings.sceneSettings.fullscreenOnPlay, UI::GetTextureID(EditorResources::GetEditorIcon(EditorIcon::FullscreenOnPlay)), { buttonSize, buttonSize }))
	{
		settings.sceneSettings.fullscreenOnPlay = !settings.sceneSettings.fullscreenOnPlay;
	}

	ImGui::SameLine();

	if (ImGui::Button("16"))
	{
		settings.sceneSettings.use16by9 = !settings.sceneSettings.use16by9;
		Resize(myViewportSize);
	}

	ImGui::SameLine();

	if (UI::ImageButton("##localWorld", UI::GetTextureID(localWorldIcon), { buttonSize, buttonSize }))
	{
		settings.sceneSettings.worldSpace = !settings.sceneSettings.worldSpace;
	}

	ImGui::SameLine();

	if (UI::ImageButtonState("##snapToGrid", settings.sceneSettings.snapToGrid, UI::GetTextureID(EditorResources::GetEditorIcon(EditorIcon::SnapGrid)), { buttonSize, buttonSize }))
	{
		settings.sceneSettings.snapToGrid = !settings.sceneSettings.snapToGrid;
	}

	// Grid Settings
	{
		UI::ScopedStyleFloat2 windowPadding{ ImGuiStyleVar_WindowPadding, { 5.f, 5.f } };

		ImGui::SetNextWindowSize({ 100.f, m_snapToGridValues.size() * 22.5f });
		if (ImGui::BeginPopupContextItem("##gridSnapValues", ImGuiPopupFlags_MouseButtonRight))
		{
			for (uint32_t i = 0; i < m_snapToGridValues.size(); i++)
			{
				std::string valueStr = Utils::RemoveTrailingZeroes(std::to_string(m_snapToGridValues[i]));
				std::string	id = valueStr + "##gridSnapValue" + std::to_string(i);

				bool selected = settings.sceneSettings.gridSnapValue == m_snapToGridValues[i];

				if (ImGui::Selectable(id.c_str(), &selected))
				{
					settings.sceneSettings.gridSnapValue = m_snapToGridValues[i];
				}
			}

			ImGui::EndPopup();
		}
		ImGui::SameLine();
	}

	if (UI::ImageButtonState("##snapRotation", settings.sceneSettings.snapRotation, UI::GetTextureID(EditorResources::GetEditorIcon(EditorIcon::SnapRotation)), { buttonSize, buttonSize }))
	{
		settings.sceneSettings.snapRotation = !settings.sceneSettings.snapRotation;
	}
	if (ImGui::BeginPopupContextItem("##rotateSnapValues", ImGuiPopupFlags_MouseButtonRight))
	{
		for (uint32_t i = 0; i < m_snapRotationValues.size(); i++)
		{
			std::string valueStr = Utils::RemoveTrailingZeroes(std::to_string(m_snapRotationValues[i]));
			std::string	id = valueStr + "##rotationSnapValue" + std::to_string(i);

			if (ImGui::Selectable(id.c_str()))
			{
				settings.sceneSettings.rotationSnapValue = m_snapRotationValues[i];
			}
		}

		ImGui::EndPopup();
	}

	ImGui::SameLine();

	if (UI::ImageButtonState("##snapScale", settings.sceneSettings.snapScale, UI::GetTextureID(EditorResources::GetEditorIcon(EditorIcon::SnapScale)), { buttonSize, buttonSize }))
	{
		settings.sceneSettings.snapScale = !settings.sceneSettings.snapScale;
	}
	if (ImGui::BeginPopupContextItem("##scaleSnapValues", ImGuiPopupFlags_MouseButtonRight))
	{
		for (uint32_t i = 0; i < m_snapScaleValues.size(); i++)
		{
			std::string valueStr = Utils::RemoveTrailingZeroes(std::to_string(m_snapScaleValues[i]));
			std::string	id = valueStr + "##scaleSnapValue" + std::to_string(i);

			if (ImGui::Selectable(id.c_str()))
			{
				settings.sceneSettings.scaleSnapValue = m_snapScaleValues[i];
			}
		}

		ImGui::EndPopup();
	}

	ImGui::SameLine();

	if (UI::ImageButtonState("##showGizmos", settings.sceneSettings.showGizmos, UI::GetTextureID(EditorResources::GetEditorIcon(EditorIcon::ShowGizmos)), { buttonSize, buttonSize }))
	{
		settings.sceneSettings.showGizmos = !settings.sceneSettings.showGizmos;
	}
	if (ImGui::BeginPopupContextItem("##gizmoSettings", ImGuiPopupFlags_MouseButtonRight))
	{
		ImGui::Selectable("Lights", &settings.sceneSettings.showLightSpheres);
		ImGui::Selectable("Entities", &settings.sceneSettings.showEntityGizmos);
		ImGui::Selectable("Bounding Spheres", &settings.sceneSettings.showBoundingSpheres);

		static const std::vector<const char*> colliderModes =
		{
			"None",
			"Selected",
			"All",
			"AllHideMesh"
		};

		UI::Combo("Collider View", *(int32_t*)&settings.sceneSettings.colliderViewMode, colliderModes);

		ImGui::Selectable("Environment Probes", &settings.sceneSettings.showEnvironmentProbes);

		static const std::vector<const char*> navmeshModes =
		{
			"None",
			"All",
			"Only"
		};

		UI::Combo("NavMesh", *(int32_t*)&settings.sceneSettings.navMeshViewMode, navmeshModes);

		ImGui::Selectable("NavMesh", &settings.sceneSettings.navMeshViewMode);
		ImGui::EndPopup();
	}

	ImGui::PopStyleVar(3);
	ImGui::End();
}

void ViewportPanel::OnEvent(Volt::Event& e)
{
	Volt::EventDispatcher dispatcher(e);
	dispatcher.Dispatch<Volt::KeyPressedEvent>(VT_BIND_EVENT_FN(ViewportPanel::OnKeyPressedEvent));
	dispatcher.Dispatch<Volt::MouseButtonPressedEvent>(VT_BIND_EVENT_FN(ViewportPanel::OnMousePressed));
	dispatcher.Dispatch<Volt::MouseButtonReleasedEvent>(VT_BIND_EVENT_FN(ViewportPanel::OnMouseReleased));

	myAnimatedPhysicsIcon.OnEvent(e);
}

bool ViewportPanel::OnMousePressed(Volt::MouseButtonPressedEvent& e)
{
	switch (e.GetMouseButton())
	{
		case VT_MOUSE_BUTTON_RIGHT:
		{
			if (myIsHovered)
			{
				ImGui::SetWindowFocus("Viewport");
			}
			break;
		}
	}

	return false;
}

bool ViewportPanel::OnKeyPressedEvent(Volt::KeyPressedEvent& e)
{
	if (!myIsHovered || Volt::Input::IsMouseButtonDown(VT_MOUSE_BUTTON_RIGHT) || ImGui::IsAnyItemActive())
	{
		return false;
	}

	const bool ctrlPressed = Volt::Input::IsKeyDown(VT_KEY_LEFT_CONTROL);

	switch (e.GetKeyCode())
	{
		case VT_KEY_W:
		{
			myGizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
			break;
		}

		case VT_KEY_E:
		{
			myGizmoOperation = ImGuizmo::OPERATION::ROTATE;
			break;
		}

		case VT_KEY_R:
		{
			myGizmoOperation = ImGuizmo::OPERATION::SCALE;
			break;
		}

		case VT_KEY_F6:
		{
			if (!myShowRenderTargets)
			{
				myShowRenderTargets = true;
				myCurrentRenderPass = 0;
			}
			else
			{
				//if (myCurrentRenderPass == mySceneRenderer->GetAllFramebuffers().size() - 1)
				//{
				//	myShowRenderTargets = false;
				//}
				//else
				//{
				//	myCurrentRenderPass++;
				//}
			}
			break;
		}

		case VT_KEY_G:
		{
			if (!ctrlPressed)
			{
				UserSettingsManager::GetSettings().sceneSettings.gridEnabled = !UserSettingsManager::GetSettings().sceneSettings.gridEnabled;
				mySceneRenderer->GetSettings().enableGrid = UserSettingsManager::GetSettings().sceneSettings.gridEnabled;
			}
			break;
		}

		case VT_KEY_END:
		{
			// NOT WORKING YET
			break;

			//auto& selected = SelectionManager::GetSelectedEntities();

			//if (selected.empty()) { break; }

			//auto& selectedTransformComp = myEditorScene->GetRegistry().GetComponent<Volt::TransformComponent>(selected[0]);

			//glm::vec3 currentPos = selectedTransformComp.position; // Get world instead of local
			//bool foundIntersect = false;
			//float bestHeight = FLT_MAX;

			//auto meshEntities = myEditorScene->GetAllEntitiesWith<Volt::MeshComponent>();

			//for (const auto& ent : meshEntities)
			//{
			//	if (ent == selected[0]) { continue; }

			//	auto& transformcomp = myEditorScene->GetRegistry().GetComponent<Volt::TransformComponent>(ent);
			//	auto& meshcomp = myEditorScene->GetRegistry().GetComponent<Volt::MeshComponent>(ent);
			//	auto mesh = Volt::AssetManager::GetAsset<Volt::Mesh>(meshcomp.handle);

			//	if (glm::distance(
			//		glm::vec3(transformcomp.position.x, 0.f, transformcomp.position.z),
			//		glm::vec3(currentPos.x / transformcomp.scale.x, 0.f, currentPos.z / transformcomp.scale.z)) > mesh->GetBoundingSphere().radius)
			//	{
			//		continue;
			//	}

			//	auto vList = mesh->GetVertices();
			//	auto iList = mesh->GetIndices();

			//	// Check submeshes

			//	for (int i = 0; i < iList.size(); i += 3)
			//	{
			//		auto p1 = vList[iList[i]].position;
			//		auto p2 = vList[iList[i + 1]].position;
			//		auto p3 = vList[iList[i + 2]].position;

			//		glm::vec3 intersectionPoint = { 0,0,0 };

			//		auto relativeScalePos = glm::vec3(
			//			currentPos.x / transformcomp.scale.x,
			//			currentPos.y,
			//			currentPos.z / transformcomp.scale.z);

			//		if (Volt::rayTriangleIntersection(relativeScalePos, glm::vec3(0.f, -1.f, 0.f), p1, p2, p3, intersectionPoint))
			//		{
			//			if (std::isnan(intersectionPoint.x) ||
			//				std::isnan(intersectionPoint.y) ||
			//				std::isnan(intersectionPoint.z) ||
			//				std::isinf(intersectionPoint.x) ||
			//				std::isinf(intersectionPoint.y) ||
			//				std::isinf(intersectionPoint.z))
			//			{
			//				continue;
			//			}

			//			if (glm::abs(currentPos.y - bestHeight) > glm::abs(currentPos.y - intersectionPoint.y))
			//			{
			//				bestHeight = intersectionPoint.y;
			//				foundIntersect = true;
			//			}
			//		}
			//	}
			//}

			//if (foundIntersect)
			//{
			//	selectedTransformComp.position.y = bestHeight;
			//}

			//break;
		}

		case VT_KEY_BACKSPACE:
		case VT_KEY_DELETE:
		{
			std::vector<Volt::Entity> entitiesToRemove;

			auto selection = SelectionManager::GetSelectedEntities();
			for (const auto& selectedEntity : selection)
			{
				Volt::Entity tempEnt = Volt::Entity(selectedEntity, myEditorScene.get());
				entitiesToRemove.push_back(tempEnt);

				SelectionManager::Deselect(tempEnt.GetId());
				SelectionManager::GetFirstSelectedRow() = -1;
				SelectionManager::GetLastSelectedRow() = -1;
			}

			Ref<ObjectStateCommand> command = CreateRef<ObjectStateCommand>(entitiesToRemove, ObjectStateAction::Delete);
			EditorCommandStack::GetInstance().PushUndo(command);

			bool shouldUpdateNavMesh = false;
			for (const auto& i : entitiesToRemove)
			{
				if (!shouldUpdateNavMesh && Sandbox::Get().CheckForUpdateNavMesh(i))
				{
					shouldUpdateNavMesh = true;
				}
				myEditorScene->RemoveEntity(i);
			}

			if (shouldUpdateNavMesh)
			{
				Sandbox::Get().BakeNavMesh();
			}

			break;
		}

		case VT_KEY_P:
		{
			auto& settings = UserSettingsManager::GetSettings().sceneSettings;
			if (!settings.showGizmos)
			{
				settings.showGizmos = true;
				settings.navMeshViewMode = NavMeshViewMode::All;
			}
			else
			{
				settings.navMeshViewMode = NavMeshViewMode::None;
			}
			break;
		}
	}

	return false;
}

bool ViewportPanel::OnMouseReleased(Volt::MouseButtonReleasedEvent& e)
{
	if (e.GetMouseButton() == VT_MOUSE_BUTTON_LEFT && !Volt::Input::IsKeyDown(VT_KEY_LEFT_ALT) && GlobalEditorStates::dragStartedInAssetBrowser)
	{
		if (myIsHovered)
		{
			SelectionManager::DeselectAll();
			SelectionManager::Select(myCreatedEntity.GetId());
		}

		myCreatedEntity = {};
		myCreatedAssetOnDrag = false;

		GlobalEditorStates::isDragging = false;
		GlobalEditorStates::dragStartedInAssetBrowser = false;
		GlobalEditorStates::dragAsset = Volt::Asset::Null();
	}

	return false;
}

void ViewportPanel::CheckDragDrop()
{
	glm::vec2 perspectiveSize = myPerspectiveBounds[1] - myPerspectiveBounds[0];

	int32_t mouseX = (int32_t)myViewportMouseCoords.x;
	int32_t mouseY = (int32_t)myViewportMouseCoords.y;

	if (mouseX < 0 || mouseY < 0 || mouseX >(int32_t)perspectiveSize.x || mouseY >(int32_t)perspectiveSize.y)
	{
		if (myCreatedAssetOnDrag && myCreatedEntity)
		{
			myEditorScene->RemoveEntity(myCreatedEntity);
			myCreatedAssetOnDrag = false;
		}

		return;
	}


	if (!GlobalEditorStates::isDragging || !GlobalEditorStates::dragStartedInAssetBrowser || myCreatedAssetOnDrag || GlobalEditorStates::dragAsset == Volt::Asset::Null())
	{
		return;
	}

	myIsInViewport = true;

	const Volt::AssetHandle handle = GlobalEditorStates::dragAsset;
	const Volt::AssetType type = Volt::AssetManager::Get().GetAssetTypeFromHandle(handle);

	switch (type)
	{
		case Volt::AssetType::Mesh:
		{
			Volt::Entity newEntity = myEditorScene->CreateEntity();

			Ref<ObjectStateCommand> command = CreateRef<ObjectStateCommand>(newEntity, ObjectStateAction::Create);
			EditorCommandStack::GetInstance().PushUndo(command);

			auto& meshComp = newEntity.AddComponent<Volt::MeshComponent>();
			auto mesh = Volt::AssetManager::GetAsset<Volt::Mesh>(handle);
			if (mesh)
			{
				meshComp.handle = mesh->handle;
			}

			newEntity.GetComponent<Volt::TagComponent>().tag = Volt::AssetManager::Get().GetFilePathFromAssetHandle(handle).stem().string();
			myCreatedEntity = newEntity;

			break;
		}

		case Volt::AssetType::MeshSource:
		{
			const std::filesystem::path meshSourcePath = Volt::AssetManager::Get().GetFilePathFromAssetHandle(handle);
			const std::filesystem::path vtMeshPath = meshSourcePath.parent_path() / (meshSourcePath.stem().string() + ".vtmesh");

			Volt::AssetHandle resultHandle = handle;
			Volt::Entity newEntity = myEditorScene->CreateEntity();

			Ref<ObjectStateCommand> command = CreateRef<ObjectStateCommand>(newEntity, ObjectStateAction::Create);
			EditorCommandStack::GetInstance().PushUndo(command);

			if (FileSystem::Exists(vtMeshPath))
			{
				Ref<Volt::Mesh> meshAsset = Volt::AssetManager::GetAsset<Volt::Mesh>(vtMeshPath);
				if (meshAsset && meshAsset->IsValid())
				{
					resultHandle = meshAsset->handle;
				}

				auto& meshComp = newEntity.AddComponent<Volt::MeshComponent>();
				auto mesh = Volt::AssetManager::GetAsset<Volt::Mesh>(resultHandle);
				if (mesh)
				{
					meshComp.handle = mesh->handle;
				}
			}
			else
			{
				myEntityToAddMesh = newEntity.GetId();

				myMeshImportData = {};
				myMeshToImport = meshSourcePath;
				myMeshImportData.destination = myMeshToImport.parent_path().string() + "\\" + myMeshToImport.stem().string() + ".vtmesh";
				UI::OpenModal("Import Mesh##viewport");
			}

			newEntity.GetComponent<Volt::TagComponent>().tag = meshSourcePath.stem().string();
			myCreatedEntity = newEntity;

			break;
		}

		case Volt::AssetType::ParticlePreset:
		{
			Volt::Entity newEntity = myEditorScene->CreateEntity();

			auto& particleEmitter = newEntity.AddComponent<Volt::ParticleEmitterComponent>();
			auto preset = Volt::AssetManager::GetAsset<Volt::ParticlePreset>(handle);
			if (preset)
			{
				particleEmitter.preset = preset->handle;
			}

			newEntity.GetComponent<Volt::TagComponent>().tag = Volt::AssetManager::Get().GetFilePathFromAssetHandle(handle).stem().string();
			myCreatedEntity = newEntity;

			break;
		}

		case Volt::AssetType::Prefab:
		{
			auto prefab = Volt::AssetManager::GetAsset<Volt::Prefab>(handle);
			if (!prefab || !prefab->IsValid())
			{
				break;
			}

			Wire::EntityId id = prefab->Instantiate(myEditorScene.get());
			Volt::Entity prefabEntity(id, myEditorScene.get());

			Ref<ObjectStateCommand> command = CreateRef<ObjectStateCommand>(prefabEntity, ObjectStateAction::Create);
			EditorCommandStack::GetInstance().PushUndo(command);
			myCreatedEntity = prefabEntity;

			break;
		}
	}

	ImGui::SetWindowFocus();
	myCreatedAssetOnDrag = true;
}

void ViewportPanel::UpdateCreatedEntityPosition()
{
	if (!myCreatedEntity)
	{
		return;
	}

	glm::vec3 dir = myEditorCameraController->GetCamera()->ScreenToWorldRay(myViewportMouseCoords, myViewportSize);
	glm::vec3 targetPos = myEditorCameraController->GetCamera()->GetPosition();

	while (targetPos.y > 0.f)
	{
		if (targetPos.y < (targetPos + dir).y)
		{
			break;
		}

		targetPos += dir;
	}

	const auto& settings = UserSettingsManager::GetSettings();

	if (settings.sceneSettings.snapToGrid)
	{
		targetPos.x = std::round(targetPos.x / settings.sceneSettings.gridSnapValue) * settings.sceneSettings.gridSnapValue;
		targetPos.z = std::round(targetPos.z / settings.sceneSettings.gridSnapValue) * settings.sceneSettings.gridSnapValue;
	}

	myCreatedEntity.SetPosition({ targetPos.x, 0.f, targetPos.z });
}

void ViewportPanel::DuplicateSelection()
{
	myEditorCameraController->ForceLooseControl();

	std::vector<Wire::EntityId> duplicated;
	for (const auto& ent : SelectionManager::GetSelectedEntities())
	{
		if (SelectionManager::IsAnyParentSelected(ent, myEditorScene))
		{
			continue;
		}

		duplicated.emplace_back(Volt::Entity::Duplicate(myEditorScene->GetRegistry(), myEditorScene->GetScriptFieldCache(), ent));
	}

	SelectionManager::DeselectAll();

	for (const auto& ent : duplicated)
	{
		SelectionManager::Select(ent);
	}
}

void ViewportPanel::HandleSingleSelect()
{
	glm::vec2 perspectiveSize = myPerspectiveBounds[1] - myPerspectiveBounds[0];

	int32_t mouseX = (int32_t)myViewportMouseCoords.x;
	int32_t mouseY = (int32_t)myViewportMouseCoords.y;

	if (mouseX >= 0 && mouseY >= 0 && mouseX < (int32_t)perspectiveSize.x && mouseY < (int32_t)perspectiveSize.y)
	{
		const auto renderScale = mySceneRenderer->GetSettings().renderScale;

		uint32_t pixelData = mySceneRenderer->GetIDImage()->ReadPixel<uint32_t>(static_cast<uint32_t>(mouseX * renderScale), static_cast<uint32_t>(mouseY * renderScale));
		const bool multiSelect = Volt::Input::IsKeyDown(VT_KEY_LEFT_SHIFT);
		const bool deselect = Volt::Input::IsKeyDown(VT_KEY_LEFT_CONTROL);

		if (!multiSelect && !deselect)
		{
			SelectionManager::DeselectAll();
		}

		if (pixelData != Wire::NullID && myEditorScene->GetRegistry().Exists(pixelData))
		{
			if (myEditorScene->GetRegistry().HasComponent<Volt::TransformComponent>(pixelData))
			{
				if (myEditorScene->GetRegistry().GetComponent<Volt::TransformComponent>(pixelData).locked)
				{
					return;
				}
			}

			if (deselect)
			{
				SelectionManager::Deselect(pixelData);
			}
			else
			{
				SelectionManager::Select(pixelData);
				EditorLibrary::Get<SceneViewPanel>()->HighlightEntity(pixelData);
			}
		}
	}
}

void ViewportPanel::HandleMultiSelect()
{
	SelectionManager::DeselectAll();

	auto* drawList = ImGui::GetWindowDrawList();

	drawList->AddRectFilled(myStartDragPos, ImGui::GetMousePos(), IM_COL32(97, 192, 255, 127));

	const glm::vec2 startDragPos = GetViewportLocalPosition(myStartDragPos);
	const glm::vec2 currentDragPos = GetViewportLocalPosition(ImGui::GetMousePos());

	int32_t minDragBoxX = (int32_t)glm::min(startDragPos.x, currentDragPos.x);
	int32_t minDragBoxY = (int32_t)glm::min(startDragPos.y, currentDragPos.y);

	int32_t maxDragBoxX = (int32_t)glm::max(startDragPos.x, currentDragPos.x);
	int32_t maxDragBoxY = (int32_t)glm::max(startDragPos.y, currentDragPos.y);

	glm::vec2 perspectiveSize = myPerspectiveBounds[1] - myPerspectiveBounds[0];

	if (minDragBoxX >= 0 && minDragBoxY >= 0 && minDragBoxX < (int32_t)perspectiveSize.x && minDragBoxY < (int32_t)perspectiveSize.y &&
		maxDragBoxX >= 0 && maxDragBoxY >= 0 && maxDragBoxX < (int32_t)perspectiveSize.x && maxDragBoxY < (int32_t)perspectiveSize.y)
	{
		auto renderScale = mySceneRenderer->GetSettings().renderScale;

		const std::vector<uint32_t> data = mySceneRenderer->GetIDImage()->ReadPixelRange<uint32_t>(
			(uint32_t)(minDragBoxX * renderScale), (uint32_t)(minDragBoxY * renderScale),
			(uint32_t)(maxDragBoxX * renderScale), (uint32_t)(maxDragBoxY * renderScale));

		for (const auto& d : data)
		{
			if (d != Wire::NullID)
			{
				SelectionManager::Select(d);
			}
		}
	}
}

void ViewportPanel::HandleSingleGizmoInteraction(const glm::mat4& avgTransform)
{
	auto firstEntity = SelectionManager::GetSelectedEntities().front();

	if (!myEditorScene->GetRegistry().HasComponent<Volt::RelationshipComponent>(firstEntity) || !myEditorScene->GetRegistry().HasComponent<Volt::TransformComponent>(firstEntity))
	{
		return;
	}

	auto& relationshipComp = myEditorScene->GetRegistry().GetComponent<Volt::RelationshipComponent>(firstEntity);
	auto& transComp = myEditorScene->GetRegistry().GetComponent<Volt::TransformComponent>(firstEntity);

	if (myMidEvent == false)
	{
		GizmoCommand::GizmoData data;
		data.positionAdress = &transComp.position;
		data.rotationAdress = &transComp.rotation;
		data.scaleAdress = &transComp.scale;
		data.previousPositionValue = transComp.position;
		data.previousRotationValue = glm::eulerAngles(transComp.rotation);
		data.previousScaleValue = transComp.scale;
		data.id = firstEntity;
		data.scene = myEditorScene;

		Ref<GizmoCommand> command = CreateRef<GizmoCommand>(data);
		EditorCommandStack::PushUndo(command);
		myMidEvent = true;
	}

	glm::mat4 averageTransform = avgTransform;

	if (relationshipComp.Parent != 0)
	{
		Volt::Entity parent(relationshipComp.Parent, myEditorScene.get());
		auto pTransform = myEditorScene->GetWorldSpaceTransform(parent);

		averageTransform = glm::inverse(pTransform) * averageTransform;
	}

	glm::vec3 p = 0.f, s = 1.f;
	glm::quat r;
	Math::Decompose(averageTransform, p, r, s);

	glm::quat delta = glm::inverse(transComp.rotation) * r;

	Volt::Entity ent{ firstEntity, myEditorScene.get() };

	ent.SetLocalPosition(p);
	ent.SetLocalRotation(transComp.rotation * delta);
	ent.SetLocalScale(s);
}

void ViewportPanel::HandleMultiGizmoInteraction(const glm::mat4& deltaTransform)
{
	std::vector<std::pair<Wire::EntityId, Volt::TransformComponent>> previousTransforms;

	for (const auto& entId : SelectionManager::GetSelectedEntities())
	{
		if (SelectionManager::IsAnyParentSelected(entId, myEditorScene))
		{
			continue;
		}

		if (!myEditorScene->GetRegistry().HasComponent<Volt::RelationshipComponent>(entId) || !myEditorScene->GetRegistry().HasComponent<Volt::TransformComponent>(entId))
		{
			continue;
		}

		auto& relationshipComp = myEditorScene->GetRegistry().GetComponent<Volt::RelationshipComponent>(entId);
		auto& transComp = myEditorScene->GetRegistry().GetComponent<Volt::TransformComponent>(entId);

		if (!myMidEvent)
		{
			previousTransforms.emplace_back(entId, transComp);
		}

		glm::mat4 entDeltaTransform = deltaTransform;

		if (relationshipComp.Parent != 0)
		{
			Volt::Entity parent(relationshipComp.Parent, myEditorScene.get());
			auto pTransform = myEditorScene->GetWorldSpaceTransform(parent);

			entDeltaTransform = glm::inverse(pTransform) * entDeltaTransform;
		}

		glm::vec3 p = 0.f, s = 1.f;
		glm::quat r;

		Math::Decompose(entDeltaTransform * transComp.GetTransform(), p, r, s);

		glm::quat delta = glm::inverse(transComp.rotation) * r;

		Volt::Entity ent{ entId, myEditorScene.get() };

		ent.SetLocalPosition(p);
		ent.SetLocalRotation(transComp.rotation * delta);
		ent.SetLocalScale(s);
	}

	if (!previousTransforms.empty())
	{
		myMidEvent = true;

		Ref<MultiGizmoCommand> command = CreateRef<MultiGizmoCommand>(myEditorScene, previousTransforms);
		EditorCommandStack::PushUndo(command);
	}
}

void ViewportPanel::UpdateModals()
{
	if (ImportState returnVal = EditorUtils::MeshImportModal("Import Mesh##viewport", myMeshImportData, myMeshToImport); returnVal == ImportState::Imported)
	{
		Volt::Entity tempEnt{ myEntityToAddMesh, myEditorScene.get() };

		auto& meshComp = tempEnt.AddComponent<Volt::MeshComponent>();
		auto mesh = Volt::AssetManager::GetAsset<Volt::Mesh>(myMeshImportData.destination);
		Volt::AssetManager::Get().SaveAsset(mesh);
		if (mesh)
		{
			meshComp.handle = mesh->handle;
		}
	}

	if (SaveReturnState returnState = EditorUtils::SaveFilePopup("Do you want to save scene?##OpenSceneViewport"); returnState != SaveReturnState::None)
	{
		if (returnState == SaveReturnState::Save)
		{
			Sandbox::Get().SaveScene();
		}

		Sandbox::Get().OpenScene(Volt::AssetManager::GetFilePathFromAssetHandle(mySceneToOpen));
		mySceneToOpen = Volt::Asset::Null();
	}
}

void ViewportPanel::HandleNonMeshDragDrop()
{
	// #TODO_Ivar: Reimplement

	if (void* ptr = UI::DragDropTarget({ "ASSET_BROWSER_ITEM" }))
	{
		const Volt::AssetHandle handle = *(const Volt::AssetHandle*)ptr;
		const Volt::AssetType type = Volt::AssetManager::Get().GetAssetTypeFromHandle(handle);

		switch (type)
		{
			case Volt::AssetType::Material:
			{
				auto material = Volt::AssetManager::GetAsset<Volt::Material>(handle);
				if (!material || !material->IsValid())
				{
					break;
				}

				glm::vec2 perspectiveSize = myPerspectiveBounds[1] - myPerspectiveBounds[0];

				int32_t mouseX = (int32_t)myViewportMouseCoords.x;
				int32_t mouseY = (int32_t)myViewportMouseCoords.y;

				if (mouseX >= 0 && mouseY >= 0 && mouseX < (int32_t)perspectiveSize.x && mouseY < (int32_t)perspectiveSize.y)
				{
					uint32_t pixelData = mySceneRenderer->GetIDImage()->ReadPixel<uint32_t>(mouseX, mouseY);

					if (myEditorScene->GetRegistry().HasComponent<Volt::MeshComponent>(pixelData))
					{
						auto& meshComponent = myEditorScene->GetRegistry().GetComponent<Volt::MeshComponent>(pixelData);
						meshComponent.overrideMaterial = material->handle;
					}
				}

				break;
			}

			case Volt::AssetType::Scene:
			{
				UI::OpenModal("Do you want to save scene?##OpenSceneViewport");
				mySceneToOpen = handle;

				break;
			}
		}
	}
}

void ViewportPanel::Resize(const glm::vec2& viewportSize)
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

	myEditorCameraController->UpdateProjection((uint32_t)myViewportSize.x, (uint32_t)myViewportSize.y);

	Volt::ViewportResizeEvent resizeEvent{ (uint32_t)myPerspectiveBounds[0].x, (uint32_t)myPerspectiveBounds[0].y, (uint32_t)myViewportSize.x, (uint32_t)myViewportSize.y };
	Volt::Application::Get().OnEvent(resizeEvent);
}

glm::vec2 ViewportPanel::GetViewportLocalPosition(const ImVec2& mousePos)
{
	auto [mx, my] = mousePos;
	mx -= myPerspectiveBounds[0].x;
	my -= myPerspectiveBounds[0].y;

	glm::vec2 perspectiveSize = myPerspectiveBounds[1] - myPerspectiveBounds[0];
	glm::vec2 result = { mx, my };

	return result;
}

glm::vec2 ViewportPanel::GetViewportLocalPosition(const glm::vec2& mousePos)
{
	float mx = mousePos.x;
	float my = mousePos.y;

	mx -= myPerspectiveBounds[0].x;
	my -= myPerspectiveBounds[0].y;

	glm::vec2 perspectiveSize = myPerspectiveBounds[1] - myPerspectiveBounds[0];
	glm::vec2 result = { mx, my };

	return result;
}

glm::mat4 ViewportPanel::CalculateAverageTransform()
{
	glm::vec3 avgTranslation;
	glm::quat avgRotation;
	glm::vec3 avgScale;

	for (const auto& ent : SelectionManager::GetSelectedEntities())
	{
		const auto trs = myEditorScene->GetWorldSpaceTRS(Volt::Entity{ ent, myEditorScene.get() });

		avgTranslation += trs.position;
		avgRotation = trs.rotation;
		avgScale += trs.scale;
	}

	avgTranslation /= (float)SelectionManager::GetSelectedCount();
	avgScale /= (float)SelectionManager::GetSelectedCount();

	return glm::translate(glm::mat4(1.f), avgTranslation) * glm::mat4_cast(avgRotation) * glm::scale(glm::mat4(1.f), avgScale);
}
