#include "sbpch.h"
#include "sbpch.h"
#include "Window/ViewportPanel.h"

#include "Sandbox/Window/SceneViewPanel.h"
#include "Sandbox/Window/GameViewPanel.h"
#include "Sandbox/Camera/EditorCameraController.h"
#include "Sandbox/Utility/SelectionManager.h"
#include "Sandbox/Utility/GlobalEditorStates.h"
#include "Sandbox/Utility/EditorResources.h"
#include "Sandbox/Utility/EditorLibrary.h"
#include "Sandbox/UserSettingsManager.h"
#include "Sandbox/Sandbox.h"

#include "Sandbox/UISystems/ModalSystem.h"
#include "Sandbox/Modals/MeshImportModal.h"

#include "Sandbox/EditorCommandStack.h"

#include "Sandbox/Utility/Theme.h"

#include <Volt/Asset/Mesh/Mesh.h>
#include <Volt/Asset/ParticlePreset.h>
#include <Volt/Asset/Prefab.h>

#include <InputModule/Input.h>
#include <InputModule/InputCodes.h>
#include <InputModule/MouseButtonCodes.h>

#include <Volt/Rendering/SceneRenderer.h>
#include <Volt/Rendering/Camera/Camera.h>

#include <Volt/Components/CoreComponents.h>
#include <Volt/Components/RenderingComponents.h>

#include <Volt/Scene/Entity.h>
#include <Volt/Scene/SceneManager.h>
#include <Volt/Asset/Serializers/SceneSerializer.h>
#include <Volt/Utility/UIUtility.h>

#include <Volt/Utility/StringUtility.h>
#include <Volt/Math/RayTriangle.h>
#include <Volt/Math/Math.h>

#include <EventSystem/EventSystem.h>
#include <WindowModule/Events/WindowEvents.h>

#include <RHIModule/Images/Image.h>

#include <Navigation/Core/NavigationSystem.h>

ViewportPanel::ViewportPanel(Ref<Volt::SceneRenderer>& sceneRenderer, Ref<Volt::Scene>& editorScene, EditorCameraController* cameraController,
	SceneState& aSceneState)
	: EditorWindow("Viewport"), m_sceneRenderer(sceneRenderer), m_editorCameraController(cameraController), m_editorScene(editorScene),
	m_sceneState(aSceneState), m_animatedPhysicsIcon("Editor/Textures/Icons/Physics/LampPhysicsAnim1.dds", 30)
{
	Open();
	m_windowFlags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
	m_midEvent = false;
	m_isFullscreenImage = true;

	RegisterListener<Volt::KeyPressedEvent>(VT_BIND_EVENT_FN(ViewportPanel::OnKeyPressedEvent));
	RegisterListener<Volt::MouseButtonPressedEvent>(VT_BIND_EVENT_FN(ViewportPanel::OnMousePressed));
	RegisterListener<Volt::MouseButtonReleasedEvent>(VT_BIND_EVENT_FN(ViewportPanel::OnMouseReleased));

	auto& meshModal = ModalSystem::AddModal<MeshImportModal>("Import Mesh##viewport");
	m_meshImportModal = meshModal.GetID();
}

void ViewportPanel::UpdateMainContent()
{
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4{ 0.07f, 0.07f, 0.07f, 1.f });

	auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
	auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
	auto viewportOffset = ImGui::GetWindowPos();

	m_perspectiveBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
	m_perspectiveBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

	m_viewportMouseCoords = GetViewportLocalPosition(ImGui::GetMousePos());

	ImVec2 viewportSize = ImGui::GetContentRegionAvail();

	if (m_viewportSize != (*(glm::vec2*)&viewportSize) && viewportSize.x > 0 && viewportSize.y > 0)
	{
		Resize({ viewportSize.x, viewportSize.y });
	}

	auto& settings = UserSettingsManager::GetSettings();

	if (!settings.sceneSettings.use16by9)
	{
		ImGui::Image(UI::GetTextureID(m_sceneRenderer->GetFinalImage()), { m_viewportSize.x, m_viewportSize.y });
	}
	else
	{
		ImGui::Image(UI::GetTextureID(m_sceneRenderer->GetFinalImage()), { m_viewportSize.x, m_viewportSize.y });
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
			bool snap = Volt::Input::IsButtonDown(Volt::InputCode::LeftControl);
			const bool duplicate = Volt::Input::IsButtonDown(Volt::InputCode::LeftAlt);

			float snapValue = 0.5f;
			if (m_gizmoOperation == ImGuizmo::ROTATE)
			{
				snap = settings.sceneSettings.snapRotation && snap ? false : settings.sceneSettings.snapRotation && !snap ? true : snap;
				snapValue = settings.sceneSettings.rotationSnapValue;
			}
			else if (m_gizmoOperation == ImGuizmo::SCALE)
			{
				snap = settings.sceneSettings.snapScale && snap ? false : settings.sceneSettings.snapScale && !snap ? true : snap;
				snapValue = settings.sceneSettings.scaleSnapValue;
			}
			else if (m_gizmoOperation == ImGuizmo::TRANSLATE)
			{
				snap = settings.sceneSettings.snapToGrid && snap ? false : settings.sceneSettings.snapToGrid && !snap ? true : snap;
				snapValue = settings.sceneSettings.gridSnapValue;
			}

			float snapValues[3] = { snapValue, snapValue, snapValue };

			ImGuizmo::MODE gizmoMode = settings.sceneSettings.worldSpace ? ImGuizmo::WORLD : ImGuizmo::LOCAL;

			ImGuizmo::SetOrthographic(false);
			ImGuizmo::SetDrawlist();

			ImGuizmo::SetRect(m_perspectiveBounds[0].x, m_perspectiveBounds[0].y, m_perspectiveBounds[1].x - m_perspectiveBounds[0].x, m_perspectiveBounds[1].y - m_perspectiveBounds[0].y);

			glm::mat4 deltaMatrix = { 1.f };

			// Temp camera is needed because 1/z messes up the gizmo while it's not in view
			const auto realCam = m_editorCameraController->GetCamera();
			Ref<Volt::Camera> tempCamera = CreateRef<Volt::Camera>(realCam->GetFieldOfView(), realCam->GetAspectRatio(), realCam->GetNearPlane(), realCam->GetFarPlane(), false);
			tempCamera->SetView(realCam->GetView());

			const auto view = tempCamera->GetView();
			const auto projection = tempCamera->GetProjection();

			if (ImGuizmo::Manipulate(
				glm::value_ptr(view),
				glm::value_ptr(projection),
				m_gizmoOperation, gizmoMode, glm::value_ptr(averageTransform), glm::value_ptr(deltaMatrix), snap ? snapValues : nullptr))
			{
				for (const auto& entId : SelectionManager::GetSelectedEntities())
				{
					auto entity = m_editorScene->GetEntityFromID(entId);
					EditorUtils::MarkEntityAndChildrenAsEdited(entity);
				}
			}

			bool wasUsedPreviousFrame = isUsing;
			isUsing = ImGuizmo::IsUsing();

			if (wasUsedPreviousFrame && !isUsing)
			{
				for (auto ent : SelectionManager::GetSelectedEntities())
				{
					if (Sandbox::Get().CheckForUpdateNavMesh(m_editorScene->GetEntityFromID(ent)))
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

		m_editorCameraController->SetControllable(IsHovered() && !isUsing);
	}
	if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && ImGui::IsWindowHovered() && !ImGuizmo::IsOver() && !Volt::Input::IsButtonDown(Volt::InputCode::LeftAlt))
	{
		m_beganClick = true;
		HandleSingleSelect();
	}

	if (m_beganClick && ImGui::IsMouseDragging(ImGuiMouseButton_Left) && !Volt::Input::IsButtonDown(Volt::InputCode::LeftControl) && !Volt::Input::IsButtonDown(Volt::InputCode::LeftShift))
	{
		m_startDragPos = ImGui::GetMousePos();
		m_isDragging = true;
		m_beganClick = false;
	}

	if (m_isDragging && Volt::Input::IsButtonDown(Volt::InputCode::LeftAlt))
	{
		m_isDragging = false;
	}

	if (m_isDragging)
	{
		HandleMultiSelect();
	}

	if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
	{
		m_isDragging = false;
		m_beganClick = false;
	}

	ImGui::PopStyleColor();

	UpdateModals();
	CheckDragDrop();
	UpdateCreatedEntityPosition();
}

void ViewportPanel::UpdateContent()
{
	if (m_midEvent && ImGui::IsMouseReleased(0))
	{
		m_midEvent = false;
	}

	UI::ScopedButtonColor transparent{ EditorTheme::Buttons::TransparentButton };
	UI::ScopedColor background{ ImGuiCol_WindowBg, EditorTheme::MiddleGreyBackground };

	ImGui::Begin("##toolbar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoTabBar);

	const uint32_t rightButtonCount = 12;
	const float buttonSize = 22.f;

	auto& settings = UserSettingsManager::GetSettings();

	Ref<Volt::Texture2D> playIcon = EditorResources::GetEditorIcon(EditorIcon::Play);
	if (m_sceneState == SceneState::Play)
	{
		playIcon = EditorResources::GetEditorIcon(EditorIcon::Stop);
	}

	if (UI::ImageButton("##play", UI::GetTextureID(playIcon), { buttonSize, buttonSize }))
	{
		static Vector<Ref<EditorWindow>> fullscreenDeactivatedWindows;

		if (m_sceneState == SceneState::Edit)
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
		else if (m_sceneState == SceneState::Play)
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

	Ref<Volt::Texture2D> physicsIcon = m_animatedPhysicsIcon.GetCurrentFrame();
	static Weak<Volt::Texture2D> physicsId = physicsIcon;

	if (ImGui::ImageButtonAnimated(UI::GetTextureID(physicsId), UI::GetTextureID(physicsIcon), { buttonSize, buttonSize }))
	{
		if (m_sceneState == SceneState::Edit)
		{
			Sandbox::Get().OnSimulationStart();
			m_animatedPhysicsIcon.Play();
		}
		else if (m_sceneState == SceneState::Simulating)
		{
			Sandbox::Get().OnSimulationStop();
			m_animatedPhysicsIcon.Stop();
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
		Resize(m_viewportSize);
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
				std::string valueStr = Utility::RemoveTrailingZeroes(std::to_string(m_snapToGridValues[i]));
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
			std::string valueStr = Utility::RemoveTrailingZeroes(std::to_string(m_snapRotationValues[i]));
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
			std::string valueStr = Utility::RemoveTrailingZeroes(std::to_string(m_snapScaleValues[i]));
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

		static const Vector<const char*> colliderModes =
		{
			"None",
			"Selected",
			"All",
			"AllHideMesh"
		};

		UI::Combo("Collider View", *(int32_t*)&settings.sceneSettings.colliderViewMode, colliderModes);

		ImGui::Selectable("Environment Probes", &settings.sceneSettings.showEnvironmentProbes);

		static const Vector<const char*> navmeshModes =
		{
			"None",
			"All",
			"Only"
		};

		UI::Combo("NavMesh", *(int32_t*)&settings.sceneSettings.navMeshViewMode, navmeshModes);

		ImGui::Selectable("NavMesh", &settings.sceneSettings.navMeshViewMode);
		ImGui::EndPopup();
	}

	ImGui::PopStyleVar(1);
	ImGui::End();
}

bool ViewportPanel::OnMousePressed(Volt::MouseButtonPressedEvent& e)
{
	switch (e.GetMouseButton())
	{
		case Volt::InputCode::Mouse_RB:
		{
			if (IsHovered())
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
	if (!IsHovered() || Volt::Input::IsButtonDown(Volt::InputCode::Mouse_RB) || ImGui::IsAnyItemActive())
	{
		return false;
	}

	const bool ctrlPressed = Volt::Input::IsButtonDown(Volt::InputCode::LeftControl);

	switch (e.GetKeyCode())
	{
		case Volt::InputCode::W:
		{
			m_gizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
			break;
		}

		case Volt::InputCode::E:
		{
			m_gizmoOperation = ImGuizmo::OPERATION::ROTATE;
			break;
		}

		case Volt::InputCode::R:
		{
			m_gizmoOperation = ImGuizmo::OPERATION::SCALE;
			break;
		}

		case Volt::InputCode::G:
		{
			if (!ctrlPressed)
			{
				UserSettingsManager::GetSettings().sceneSettings.gridEnabled = !UserSettingsManager::GetSettings().sceneSettings.gridEnabled;
				//mySceneRenderer->GetSettings().enableGrid = UserSettingsManager::GetSettings().sceneSettings.gridEnabled;
			}
			break;
		}

		case Volt::InputCode::End:
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

		case Volt::InputCode::Backspace:
		case Volt::InputCode::Delete:
		{
			Vector<Volt::Entity> entitiesToRemove;

			auto selection = SelectionManager::GetSelectedEntities();
			for (const auto& selectedEntity : selection)
			{
				Volt::Entity tempEnt = m_editorScene->GetEntityFromID(selectedEntity);
				entitiesToRemove.push_back(tempEnt);

				SelectionManager::Deselect(tempEnt.GetID());
				SelectionManager::GetFirstSelectedRow() = -1;
				SelectionManager::GetLastSelectedRow() = -1;
			}

			Ref<ObjectStateCommand> command = CreateRef<ObjectStateCommand>(entitiesToRemove, ObjectStateAction::Delete);
			EditorCommandStack::GetInstance().PushUndo(command);

			bool shouldUpdateNavMesh = false;
			for (const auto& entity : entitiesToRemove)
			{
				if (!entity)
				{
					continue;
				}

				if (!shouldUpdateNavMesh && Sandbox::Get().CheckForUpdateNavMesh(entity))
				{
					shouldUpdateNavMesh = true;
				}
				m_editorScene->DestroyEntity(entity);
			}

			if (shouldUpdateNavMesh)
			{
				Sandbox::Get().BakeNavMesh();
			}

			break;
		}

		case Volt::InputCode::P:
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
	if (e.GetMouseButton() == Volt::InputCode::Mouse_LB && !Volt::Input::IsButtonDown(Volt::InputCode::LeftAlt) && GlobalEditorStates::dragStartedInAssetBrowser)
	{
		if (IsHovered())
		{
			SelectionManager::DeselectAll();
			SelectionManager::Select(m_createdEntity.GetID());
		}

		m_createdEntity = {};
		m_createdAssetOnDrag = false;

		GlobalEditorStates::isDragging = false;
		GlobalEditorStates::dragStartedInAssetBrowser = false;
		GlobalEditorStates::dragAsset = Volt::Asset::Null();
	}

	return false;
}

void ViewportPanel::OnClose()
{
	m_animatedPhysicsIcon.SetIsEnabled(false);
}

void ViewportPanel::OnOpen()
{
	m_animatedPhysicsIcon.SetIsEnabled(true);
}

void ViewportPanel::CheckDragDrop()
{
	glm::vec2 perspectiveSize = m_perspectiveBounds[1] - m_perspectiveBounds[0];

	int32_t mouseX = (int32_t)m_viewportMouseCoords.x;
	int32_t mouseY = (int32_t)m_viewportMouseCoords.y;

	if (mouseX < 0 || mouseY < 0 || mouseX >(int32_t)perspectiveSize.x || mouseY >(int32_t)perspectiveSize.y)
	{
		if (m_createdAssetOnDrag && m_createdEntity)
		{
			m_editorScene->DestroyEntity(m_createdEntity);
			m_createdAssetOnDrag = false;
		}

		return;
	}


	if (!GlobalEditorStates::isDragging || !GlobalEditorStates::dragStartedInAssetBrowser || m_createdAssetOnDrag || GlobalEditorStates::dragAsset == Volt::Asset::Null())
	{
		return;
	}

	m_isInViewport = true;

	const Volt::AssetHandle handle = GlobalEditorStates::dragAsset;
	const AssetType type = Volt::AssetManager::GetAssetTypeFromHandle(handle);

	if (type == AssetTypes::Mesh)
	{
		Volt::Entity newEntity = m_editorScene->CreateEntity();

		Ref<ObjectStateCommand> command = CreateRef<ObjectStateCommand>(newEntity, ObjectStateAction::Create);
		EditorCommandStack::GetInstance().PushUndo(command);

		auto& meshComp = newEntity.AddComponent<Volt::MeshComponent>();
		auto mesh = Volt::AssetManager::GetAsset<Volt::Mesh>(handle);
		if (mesh)
		{
			meshComp.handle = mesh->handle;
		}

		newEntity.GetComponent<Volt::TagComponent>().tag = Volt::AssetManager::GetFilePathFromAssetHandle(handle).stem().string();

		SelectionManager::DeselectAll();
		SelectionManager::Select(newEntity.GetID());

		m_createdEntity = newEntity;

		Volt::MeshComponent::OnMemberChanged(Volt::MeshComponent::MeshEntity(newEntity.GetScene()->GetEntityHelperFromEntityID(newEntity.GetID())));
	}
	else if (type == AssetTypes::MeshSource)
	{
		const std::filesystem::path meshSourcePath = Volt::AssetManager::GetFilePathFromAssetHandle(handle);
		const std::filesystem::path vtMeshPath = meshSourcePath.parent_path() / (meshSourcePath.stem().string() + ".vtasset");

		Volt::AssetHandle resultHandle = handle;
		Volt::Entity newEntity = m_editorScene->CreateEntity();

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
			Volt::MeshComponent::OnMemberChanged(Volt::MeshComponent::MeshEntity(newEntity.GetScene()->GetEntityHelperFromEntityID(newEntity.GetID())));
		}
		else
		{
			m_entityToAddMesh = newEntity.GetID();

			ModalSystem::GetModal<MeshImportModal>(m_meshImportModal).SetImportMeshes({ meshSourcePath });
			ModalSystem::GetModal<MeshImportModal>(m_meshImportModal).Open();
		}

		newEntity.GetComponent<Volt::TagComponent>().tag = meshSourcePath.stem().string();
		m_createdEntity = newEntity;
	}
	else if (type == AssetTypes::ParticlePreset)
	{
		Volt::Entity newEntity = m_editorScene->CreateEntity();

		auto& particleEmitter = newEntity.AddComponent<Volt::ParticleEmitterComponent>();
		auto preset = Volt::AssetManager::GetAsset<Volt::ParticlePreset>(handle);
		if (preset)
		{
			particleEmitter.preset = preset->handle;
		}

		newEntity.GetComponent<Volt::TagComponent>().tag = Volt::AssetManager::GetFilePathFromAssetHandle(handle).stem().string();
		m_createdEntity = newEntity;
	}

	ImGui::SetWindowFocus();
	m_createdAssetOnDrag = true;
}

void ViewportPanel::UpdateCreatedEntityPosition()
{
	if (!m_createdEntity)
	{
		return;
	}

	glm::vec3 dir = m_editorCameraController->GetCamera()->ScreenToWorldRay(m_viewportMouseCoords, m_viewportSize);
	glm::vec3 targetPos = m_editorCameraController->GetCamera()->GetPosition();

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

	m_createdEntity.SetPosition({ targetPos.x, 0.f, targetPos.z });
}

void ViewportPanel::DuplicateSelection()
{
	m_editorCameraController->ForceDisable();

	Vector<Volt::Entity> duplicated;
	for (const auto& ent : SelectionManager::GetSelectedEntities())
	{
		if (SelectionManager::IsAnyParentSelected(ent, m_editorScene))
		{
			continue;
		}

		auto duplicatedEntity = Volt::Entity::Duplicate(m_editorScene->GetEntityFromID(ent));
		duplicatedEntity.SetTag(EditorUtils::GetDuplicatedNameFromEntity(m_editorScene->GetEntityFromID(ent)));

		duplicated.emplace_back(duplicatedEntity);
	}

	SelectionManager::DeselectAll();

	for (const auto& ent : duplicated)
	{
		SelectionManager::Select(ent.GetID());
	}
}

void ViewportPanel::HandleSingleSelect()
{
	glm::vec2 perspectiveSize = m_perspectiveBounds[1] - m_perspectiveBounds[0];

	int32_t mouseX = (int32_t)m_viewportMouseCoords.x;
	int32_t mouseY = (int32_t)m_viewportMouseCoords.y;

	if (mouseX >= 0 && mouseY >= 0 && mouseX < (int32_t)perspectiveSize.x && mouseY < (int32_t)perspectiveSize.y)
	{
		//const auto renderScale = m_sceneRenderer->GetSettings().renderScale;
		const float renderScale = 1.f;
		if (!m_sceneRenderer->GetObjectIDImage())
		{
			return;
		}

		uint32_t pixelData = m_sceneRenderer->GetObjectIDImage()->ReadPixel<uint32_t>(static_cast<uint32_t>(mouseX * renderScale), static_cast<uint32_t>(mouseY * renderScale), 0u);
		const bool multiSelect = Volt::Input::IsButtonDown(Volt::InputCode::LeftShift);
		const bool deselect = Volt::Input::IsButtonDown(Volt::InputCode::LeftControl);

		if (!multiSelect && !deselect)
		{
			SelectionManager::DeselectAll();
		}

		Volt::Entity entity = m_editorScene->GetEntityFromID(pixelData);

		if (entity.IsValid())
		{
			if (entity.HasComponent<Volt::TransformComponent>())
			{
				if (entity.GetComponent<Volt::TransformComponent>().locked)
				{
					return;
				}
			}

			if (deselect)
			{
				SelectionManager::Deselect(entity.GetID());
			}
			else
			{
				SelectionManager::Select(entity.GetID());
				EditorLibrary::Get<SceneViewPanel>()->HighlightEntity(entity);
			}
		}
	}
}

void ViewportPanel::HandleMultiSelect()
{
	SelectionManager::DeselectAll();

	auto* drawList = ImGui::GetWindowDrawList();

	drawList->AddRectFilled(m_startDragPos, ImGui::GetMousePos(), IM_COL32(97, 192, 255, 127));

	//const glm::vec2 startDragPos = GetViewportLocalPosition(myStartDragPos);
	//const glm::vec2 currentDragPos = GetViewportLocalPosition(ImGui::GetMousePos());

	//int32_t minDragBoxX = (int32_t)glm::min(startDragPos.x, currentDragPos.x);
	//int32_t minDragBoxY = (int32_t)glm::min(startDragPos.y, currentDragPos.y);

	//int32_t maxDragBoxX = (int32_t)glm::max(startDragPos.x, currentDragPos.x);
	//int32_t maxDragBoxY = (int32_t)glm::max(startDragPos.y, currentDragPos.y);

	//glm::vec2 perspectiveSize = myPerspectiveBounds[1] - myPerspectiveBounds[0];

	/*if (minDragBoxX >= 0 && minDragBoxY >= 0 && minDragBoxX < (int32_t)perspectiveSize.x && minDragBoxY < (int32_t)perspectiveSize.y &&
		maxDragBoxX >= 0 && maxDragBoxY >= 0 && maxDragBoxX < (int32_t)perspectiveSize.x && maxDragBoxY < (int32_t)perspectiveSize.y)
	{
		auto renderScale = m_sceneRenderer->GetSettings().renderScale;

		const Vector<uint32_t> data = m_sceneRenderer->GetIDImage()->ReadPixelRange<uint32_t>(
			(uint32_t)(minDragBoxX * renderScale), (uint32_t)(minDragBoxY * renderScale),
			(uint32_t)(maxDragBoxX * renderScale), (uint32_t)(maxDragBoxY * renderScale));

		for (const auto& d : data)
		{
			if (d != Volt::Entity::NullID())
			{
				SelectionManager::Select(static_cast<Volt::EntityID>(d));
			}
		}
	}*/
}

void ViewportPanel::HandleSingleGizmoInteraction(const glm::mat4& avgTransform)
{
	auto firstEntityId = SelectionManager::GetSelectedEntities().front();
	Volt::Entity entity = m_editorScene->GetEntityFromID(firstEntityId);

	if (!entity.HasComponent<Volt::RelationshipComponent>() || !entity.HasComponent<Volt::TransformComponent>())
	{
		return;
	}

	auto& relationshipComp = entity.GetComponent<Volt::RelationshipComponent>();
	auto& transComp = entity.GetComponent<Volt::TransformComponent>();

	if (m_midEvent == false)
	{
		GizmoCommand::GizmoData data;
		data.positionAdress = &transComp.position;
		data.rotationAdress = &transComp.rotation;
		data.scaleAdress = &transComp.scale;
		data.previousPositionValue = transComp.position;
		data.previousRotationValue = glm::eulerAngles(transComp.rotation);
		data.previousScaleValue = transComp.scale;
		data.id = entity.GetID();
		data.scene = m_editorScene;

		Ref<GizmoCommand> command = CreateRef<GizmoCommand>(data);
		EditorCommandStack::PushUndo(command);
		m_midEvent = true;
	}

	glm::mat4 averageTransform = avgTransform;

	Volt::Entity parent = m_editorScene->GetEntityFromID(relationshipComp.parent);
	if (parent)
	{
		auto pTransform = parent.GetTransform();
		averageTransform = glm::inverse(pTransform) * averageTransform;
	}

	glm::vec3 p = 0.f, s = 1.f;
	glm::quat r;
	Math::Decompose(averageTransform, p, r, s);

	glm::quat delta = glm::inverse(transComp.rotation) * r;

	entity.SetLocalPosition(p);
	entity.SetLocalRotation(transComp.rotation * delta);
	entity.SetLocalScale(s);
}

void ViewportPanel::HandleMultiGizmoInteraction(const glm::mat4& deltaTransform)
{
	Vector<std::pair<Volt::EntityID, Volt::TransformComponent>> previousTransforms;

	for (const auto& entId : SelectionManager::GetSelectedEntities())
	{
		Volt::Entity entity = m_editorScene->GetEntityFromID(entId);

		if (SelectionManager::IsAnyParentSelected(entId, m_editorScene))
		{
			continue;
		}

		if (!entity.HasComponent<Volt::RelationshipComponent>() || !entity.HasComponent<Volt::TransformComponent>())
		{
			continue;
		}

		auto& relationshipComp = entity.GetComponent<Volt::RelationshipComponent>();
		auto& transComp = entity.GetComponent<Volt::TransformComponent>();

		if (!m_midEvent)
		{
			previousTransforms.emplace_back(entId, transComp);
		}

		glm::mat4 entDeltaTransform = deltaTransform;

		if (relationshipComp.parent != Volt::Entity::NullID())
		{
			Volt::Entity parent = m_editorScene->GetEntityFromID(relationshipComp.parent);
			auto pTransform = parent.GetTransform();

			entDeltaTransform = glm::inverse(pTransform) * entDeltaTransform;
		}

		glm::vec3 p = 0.f, s = 1.f;
		glm::quat r;

		Math::Decompose(entDeltaTransform * transComp.GetTransform(), p, r, s);

		glm::quat delta = glm::inverse(transComp.rotation) * r;

		entity.SetLocalPosition(p);
		entity.SetLocalRotation(transComp.rotation * delta);
		entity.SetLocalScale(s);
	}

	if (!previousTransforms.empty())
	{
		m_midEvent = true;

		Ref<MultiGizmoCommand> command = CreateRef<MultiGizmoCommand>(m_editorScene, previousTransforms);
		EditorCommandStack::PushUndo(command);
	}
}

void ViewportPanel::UpdateModals()
{
	//if (ImportState returnVal = EditorUtils::MeshImportModal("Import Mesh##viewport", myMeshImportData, myMeshToImport); returnVal == ImportState::Imported)
	//{
	//	Volt::Entity tempEnt{ myEntityToAddMesh, myEditorScene.get() };

	//	auto& meshComp = tempEnt.AddComponent<Volt::MeshComponent>();
	//	auto mesh = Volt::AssetManager::GetAsset<Volt::Mesh>(myMeshImportData.destination);
	//	Volt::AssetManager::Get().SaveAsset(mesh);
	//	if (mesh)
	//	{
	//		meshComp.handle = mesh->handle;
	//	}
	//}

	if (SaveReturnState returnState = EditorUtils::SaveFilePopup("Do you want to save scene?##OpenSceneViewport"); returnState != SaveReturnState::None)
	{
		if (returnState == SaveReturnState::Save)
		{
			Sandbox::Get().SaveScene();
		}

		Sandbox::Get().OpenScene(Volt::AssetManager::GetFilePathFromAssetHandle(m_sceneToOpen));
		m_sceneToOpen = Volt::Asset::Null();
	}
}

void ViewportPanel::HandleNonMeshDragDrop()
{
	if (void* ptr = UI::DragDropTarget({ "ASSET_BROWSER_ITEM" }))
	{
		const Volt::AssetHandle handle = *(const Volt::AssetHandle*)ptr;
		const AssetType type = Volt::AssetManager::GetAssetTypeFromHandle(handle);

		if (type == AssetTypes::Material)
		{
			//auto material = Volt::AssetManager::GetAsset<Volt::Material>(handle);
				//if (!material || !material->IsValid())
				//{
				//	break;
				//}

				//glm::vec2 perspectiveSize = myPerspectiveBounds[1] - myPerspectiveBounds[0];

				//int32_t mouseX = (int32_t)myViewportMouseCoords.x;
				//int32_t mouseY = (int32_t)myViewportMouseCoords.y;

				/*if (mouseX >= 0 && mouseY >= 0 && mouseX < (int32_t)perspectiveSize.x && mouseY < (int32_t)perspectiveSize.y)
				{
					uint32_t pixelData = m_sceneRenderer->GetIDImage()->ReadPixel<uint32_t>(mouseX, mouseY);
					Volt::Entity entity{ pixelData, m_editorScene };

					if (entity.HasComponent<Volt::MeshComponent>())
					{
						auto& meshComponent = entity.GetComponent<Volt::MeshComponent>();
						meshComponent.material = material->handle;
					}
				}*/
		}
		else if (type == AssetTypes::Scene)
		{
			UI::OpenModal("Do you want to save scene?##OpenSceneViewport");
			m_sceneToOpen = handle;
		}
	}
}

void ViewportPanel::Resize(const glm::vec2& viewportSize)
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

	m_editorCameraController->UpdateProjection((uint32_t)m_viewportSize.x, (uint32_t)m_viewportSize.y);

	Volt::ViewportResizeEvent resizeEvent{ (uint32_t)m_perspectiveBounds[0].x, (uint32_t)m_perspectiveBounds[0].y, (uint32_t)m_viewportSize.x, (uint32_t)m_viewportSize.y };
	Volt::EventSystem::DispatchEvent(resizeEvent);
}

glm::vec2 ViewportPanel::GetViewportLocalPosition(const ImVec2& mousePos)
{
	auto [mx, my] = mousePos;
	mx -= m_perspectiveBounds[0].x;
	my -= m_perspectiveBounds[0].y;

	glm::vec2 perspectiveSize = m_perspectiveBounds[1] - m_perspectiveBounds[0];
	glm::vec2 result = { mx, my };

	return result;
}

glm::vec2 ViewportPanel::GetViewportLocalPosition(const glm::vec2& mousePos)
{
	float mx = mousePos.x;
	float my = mousePos.y;

	mx -= m_perspectiveBounds[0].x;
	my -= m_perspectiveBounds[0].y;

	glm::vec2 perspectiveSize = m_perspectiveBounds[1] - m_perspectiveBounds[0];
	glm::vec2 result = { mx, my };

	return result;
}

glm::mat4 ViewportPanel::CalculateAverageTransform()
{
	glm::vec3 avgTranslation = 0.f;
	glm::quat avgRotation = glm::identity<glm::quat>();
	glm::vec3 avgScale = 0.f;

	for (const auto& ent : SelectionManager::GetSelectedEntities())
	{
		const auto trs = m_editorScene->GetEntityWorldTQS(m_editorScene->GetEntityFromID(ent));

		avgTranslation += trs.translation;
		avgRotation = trs.rotation;
		avgScale += trs.scale;
	}

	avgTranslation /= (float)SelectionManager::GetSelectedCount();
	avgScale /= (float)SelectionManager::GetSelectedCount();

	return glm::translate(glm::mat4(1.f), avgTranslation) * glm::mat4_cast(avgRotation) * glm::scale(glm::mat4(1.f), avgScale);
}
