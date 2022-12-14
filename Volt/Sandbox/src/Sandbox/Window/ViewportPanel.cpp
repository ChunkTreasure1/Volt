#include "sbpch.h"
#include "ViewportPanel.h"

#include "Sandbox/Camera/EditorCameraController.h"
#include "Sandbox/Utility/EditorIconLibrary.h"
#include "Sandbox/Utility/SelectionManager.h"
#include "Sandbox/Utility/GlobalEditorStates.h"
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

#include <Volt/Rendering/SceneRenderer.h>
#include <Volt/Rendering/Framebuffer.h>
#include <Volt/Rendering/Camera/Camera.h>
#include <Volt/Rendering/Framebuffer.h>
#include <Volt/Rendering/Renderer.h>
#include <Volt/Rendering/Texture/Image2D.h>

#include <Volt/Scene/Entity.h>
#include <Volt/Utility/UIUtility.h>
#include <Volt/Utility/Math.h>

#include <Volt/Utility/StringUtility.h>

#include "Sandbox/EditorCommandStack.h"

ViewportPanel::ViewportPanel(Ref<Volt::SceneRenderer>& sceneRenderer, Ref<Volt::Scene>& editorScene, EditorCameraController* cameraController,
	SceneState& aSceneState)
	: EditorWindow("Viewport"), mySceneRenderer(sceneRenderer), myEditorCameraController(cameraController), myEditorScene(editorScene),
	mySceneState(aSceneState), myAnimatedPhysicsIcon("Editor/Textures/Icons/Physics/LampPhysicsAnim1.dds", 30)
{
	myIsOpen = true;
	myWindowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
	myMidEvent = false;
}

void ViewportPanel::UpdateMainContent()
{
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 0.f, 0.f });
	ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2{ 0.f, 0.f });
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0.f, 0.f });
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4{ 0.07f, 0.07f, 0.07f, 1.f });

	auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
	auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
	auto viewportOffset = ImGui::GetWindowPos();

	myPerspectiveBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
	myPerspectiveBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

	myViewportMouseCoords = GetViewportLocalPosition(ImGui::GetMousePos());

	ImVec2 viewportSize = ImGui::GetContentRegionAvail();

	if (myViewportSize != (*(gem::vec2*)&viewportSize) && viewportSize.x > 0 && viewportSize.y > 0)
	{
		Resize({ viewportSize.x, viewportSize.y });
	}

	auto& settings = UserSettingsManager::GetSettings();

	if (!myShowRenderTargets)
	{
		if (!settings.sceneSettings.use16by9)
		{
			ImGui::Image(UI::GetTextureID(mySceneRenderer->GetFinalFramebuffer()->GetColorAttachment(0)), { myViewportSize.x, myViewportSize.y });
		}
		else
		{
			ImGui::Image(UI::GetTextureID(mySceneRenderer->GetFinalFramebuffer()->GetColorAttachment(0)), { myViewportSize.x, myViewportSize.y });
		}

	}
	else
	{
		const auto& passes = mySceneRenderer->GetAllFramebuffers();
		const auto& [name, framebuffer] = passes.at(myCurrentRenderPass);

		const uint32_t rowCount = (uint32_t)std::sqrt(framebuffer->GetSpecification().attachments.size()) + 1;

		ImVec2 perViewportSize = { myViewportSize.x / rowCount, myViewportSize.y / rowCount };

		ImGui::TextUnformatted(name.c_str());
		for (uint32_t column = 0, i = 0; i < (uint32_t)framebuffer->GetSpecification().attachments.size(); i++)
		{
			const auto& att = framebuffer->GetSpecification().attachments.at(i);
			if (Volt::Utility::IsDepthFormat(att.format) || Volt::Utility::IsTypeless(att.format))
			{
				continue;
			}

			const ImVec2 startPos = ImGui::GetCursorPos();
			ImGui::Image(UI::GetTextureID(framebuffer->GetColorAttachment(i)), perViewportSize);

			column++;
			if (column < rowCount)
			{
				ImGui::SameLine();
			}
			else
			{
				column = 0;
			}
			const ImVec2 endPos = ImGui::GetCursorPos();

			ImGui::SetCursorPos(startPos);
			ImGui::TextUnformatted(framebuffer->GetColorAttachment(i)->GetSpecification().debugName.c_str());
			ImGui::SetCursorPos(endPos);
		}
	}

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

				gem::vec2 perspectiveSize = myPerspectiveBounds[1] - myPerspectiveBounds[0];

				int32_t mouseX = (int32_t)myViewportMouseCoords.x;
				int32_t mouseY = (int32_t)myViewportMouseCoords.y;

				if (mouseX >= 0 && mouseY >= 0 && mouseX < (int32_t)perspectiveSize.x && mouseY < (int32_t)perspectiveSize.y)
				{
					uint32_t pixelData = mySceneRenderer->GetSelectionFramebuffer()->ReadPixel<uint32_t>(3, mouseX, mouseY);

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

	// Gizmo
	{
		static gem::mat4 averageTransform = gem::mat4(1.f);
		static gem::mat4 averageStartTransform = gem::mat4(1.f);

		static bool hasDuplicated = false;
		static bool isUsing = false;

		if (SelectionManager::IsAnySelected() && mySceneState != SceneState::Play)
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

			ImGuizmo::Manipulate(
				gem::value_ptr(myEditorCameraController->GetCamera()->GetView()),
				gem::value_ptr(myEditorCameraController->GetCamera()->GetProjection()),
				myGizmoOperation, gizmoMode, gem::value_ptr(averageTransform), nullptr, snap ? snapValues : nullptr);


			isUsing = ImGuizmo::IsUsing();

			if (isUsing)
			{
				averageStartTransform = CalculateAverageTransform();

				if (duplicate && !hasDuplicated)
				{
					myEditorCameraController->ForceLooseControl();

					std::vector<Wire::EntityId> duplicated;
					for (const auto& ent : SelectionManager::GetSelectedEntities())
					{
						duplicated.emplace_back(Volt::Entity::Duplicate(myEditorScene->GetRegistry(), ent));
					}

					SelectionManager::DeselectAll();

					for (const auto& ent : duplicated)
					{
						SelectionManager::Select(ent);
					}

					hasDuplicated = true;
				}
				else if (averageTransform != averageStartTransform)
				{
					if (SelectionManager::GetSelectedCount() == 1)
					{
						auto& relationshipComp = myEditorScene->GetRegistry().GetComponent<Volt::RelationshipComponent>(SelectionManager::GetSelectedEntities().front());
						auto& transComp = myEditorScene->GetRegistry().GetComponent<Volt::TransformComponent>(SelectionManager::GetSelectedEntities().front());

						if (myMidEvent == false)
						{
							GizmoCommand<gem::vec3>::GizmoData data;
							data.positionAdress = &transComp.position;
							data.rotationAdress = &transComp.rotation;
							data.scaleAdress = &transComp.scale;
							data.previousPositionValue = transComp.position;
							data.previousRotationValue = transComp.rotation;
							data.previousScaleValue = transComp.scale;

							Ref<GizmoCommand<gem::vec3>> command = CreateRef<GizmoCommand<gem::vec3>>(data);
							EditorCommandStack::PushUndo(command);
							myMidEvent = true;
						}

						if (relationshipComp.Parent != 0)
						{
							Volt::Entity parent(relationshipComp.Parent, myEditorScene.get());
							auto pTransform = myEditorScene->GetWorldSpaceTransform(parent);

							averageTransform = gem::inverse(pTransform) * averageTransform;
						}

						gem::vec3 p = 0.f, r = 0.f, s = 1.f;
						Volt::Math::DecomposeTransform(averageTransform, p, r, s);

						gem::vec3 deltaRot = r - transComp.rotation;

						transComp.position = p;
						transComp.rotation += deltaRot;
						transComp.scale = s;
					}
					else
					{
						gem::vec3 newPos, newRot, newScale, oldPos, oldRot, oldScale;

						gem::decompose(averageTransform, newPos, newRot, newScale);
						gem::decompose(averageStartTransform, oldPos, oldRot, oldScale);

						gem::vec3 deltaPos = newPos - oldPos;
						gem::vec3 deltaScale = newScale - oldScale;

						for (const auto& ent : SelectionManager::GetSelectedEntities())
						{
							auto& relationshipComp = myEditorScene->GetRegistry().GetComponent<Volt::RelationshipComponent>(ent);
							auto& transComp = myEditorScene->GetRegistry().GetComponent<Volt::TransformComponent>(ent);

							gem::vec3 p = 0.f, r = 0.f, s = 1.f;
							Volt::Math::DecomposeTransform(averageTransform, p, r, s);

							//gem::vec3 deltaRot = r - transComp.rotation;

							transComp.position += deltaPos;
							//transComp.rotation += deltaRot;
							transComp.scale += deltaScale;
						}
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
	if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && ImGui::IsWindowHovered() && !ImGuizmo::IsOver() && mySceneState != SceneState::Play && !Volt::Input::IsKeyDown(VT_KEY_LEFT_ALT))
	{
		myBeganClick = true;

		gem::vec2 perspectiveSize = myPerspectiveBounds[1] - myPerspectiveBounds[0];

		int32_t mouseX = (int32_t)myViewportMouseCoords.x;
		int32_t mouseY = (int32_t)myViewportMouseCoords.y;

		if (mouseX >= 0 && mouseY >= 0 && mouseX < (int32_t)perspectiveSize.x && mouseY < (int32_t)perspectiveSize.y)
		{
			uint32_t pixelData = mySceneRenderer->GetSelectionFramebuffer()->ReadPixel<uint32_t>(3, mouseX, mouseY);
			const bool multiSelect = Volt::Input::IsKeyDown(VT_KEY_LEFT_SHIFT);
			const bool deselect = Volt::Input::IsKeyDown(VT_KEY_LEFT_CONTROL);

			if (!multiSelect && !deselect)
			{
				SelectionManager::DeselectAll();
			}

			if (pixelData != Wire::NullID)
			{
				if (deselect)
				{
					SelectionManager::Deselect(pixelData);
				}
				else
				{
					SelectionManager::Select(pixelData);
				}
			}
		}
	}

	if (myBeganClick && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
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
		SelectionManager::DeselectAll();

		auto* drawList = ImGui::GetWindowDrawList();

		drawList->AddRectFilled(myStartDragPos, ImGui::GetMousePos(), IM_COL32(97, 192, 255, 127));

		const gem::vec2 startDragPos = GetViewportLocalPosition(myStartDragPos);
		const gem::vec2 currentDragPos = GetViewportLocalPosition(ImGui::GetMousePos());

		int32_t minDragBoxX = (int32_t)gem::min(startDragPos.x, currentDragPos.x);
		int32_t minDragBoxY = (int32_t)gem::min(startDragPos.y, currentDragPos.y);

		int32_t maxDragBoxX = (int32_t)gem::max(startDragPos.x, currentDragPos.x);
		int32_t maxDragBoxY = (int32_t)gem::max(startDragPos.y, currentDragPos.y);

		gem::vec2 perspectiveSize = myPerspectiveBounds[1] - myPerspectiveBounds[0];

		if (minDragBoxX >= 0 && minDragBoxY >= 0 && minDragBoxX < (int32_t)perspectiveSize.x && minDragBoxY < (int32_t)perspectiveSize.y &&
			maxDragBoxX >= 0 && maxDragBoxY >= 0 && maxDragBoxX < (int32_t)perspectiveSize.x && maxDragBoxY < (int32_t)perspectiveSize.y)
		{
			std::vector<uint32_t> data = mySceneRenderer->GetSelectionFramebuffer()->ReadPixelRange<uint32_t>(3, minDragBoxX, minDragBoxY, maxDragBoxX, maxDragBoxY);
			for (const auto& d : data)
			{
				if (d != Wire::NullID)
				{
					SelectionManager::Select(d);
				}
			}
		}
	}

	if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
	{
		myIsDragging = false;
		myBeganClick = false;
	}

	ImGui::PopStyleColor();
	ImGui::PopStyleVar(3);

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

		if (myEditorScene->handle == mySceneToOpen)
		{
			Volt::AssetManager::Get().ReloadAsset(myEditorScene->handle);
		}

		myEditorScene = Volt::AssetManager::GetAsset<Volt::Scene>(mySceneToOpen);
		mySceneRenderer = CreateRef<Volt::SceneRenderer>(myEditorScene);

		Volt::OnSceneLoadedEvent loadEvent{ myEditorScene };
		Volt::Application::Get().OnEvent(loadEvent);

		mySceneToOpen = Volt::Asset::Null();
	}

	CheckDragDrop();
	UpdateCreatedEntityPosition();
}

void ViewportPanel::UpdateContent()
{
	if (myMidEvent && Volt::Input::IsMouseButtonReleased(VT_MOUSE_BUTTON_LEFT))
	{
		myMidEvent = false;
	}

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 2.f));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0.f, 0.f));
	UI::ScopedColor button(ImGuiCol_Button, { 0.f, 0.f, 0.f, 0.f });
	UI::ScopedColor hovered(ImGuiCol_ButtonHovered, { 0.3f, 0.305f, 0.31f, 0.5f });
	UI::ScopedColor active(ImGuiCol_ButtonActive, { 0.5f, 0.505f, 0.51f, 0.5f });

	ImGui::Begin("##toolbar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

	const uint32_t rightButtonCount = 12;
	const float buttonSize = 22.f;

	auto& settings = UserSettingsManager::GetSettings();

	float size = ImGui::GetWindowHeight() - 4.f;
	Ref<Volt::Texture2D> playIcon = EditorIconLibrary::GetIcon(EditorIcon::Play);
	if (mySceneState == SceneState::Play)
	{
		playIcon = EditorIconLibrary::GetIcon(EditorIcon::Stop);
	}

	if (UI::ImageButton("##play", UI::GetTextureID(playIcon), { buttonSize, buttonSize }))
	{
		if (mySceneState == SceneState::Edit)
		{
			Sandbox::Get().OnScenePlay();
			Volt::ViewportResizeEvent resizeEvent{ (uint32_t)myPerspectiveBounds[0].x, (uint32_t)myPerspectiveBounds[0].y, (uint32_t)myViewportSize.x, (uint32_t)myViewportSize.y };
			Volt::Application::Get().OnEvent(resizeEvent);
			
			if (settings.sceneSettings.fullscreenOnPlay)
			{
				for (const auto& window : Sandbox::Get().GetEditorWindows())
				{
					if (window->GetTitle() == "Scene View" || 
						window->GetTitle() == "Asset Browser##Main" ||
						window->GetTitle() == "Properties")
					{
						const_cast<bool&>(window->IsOpen()) = false;
					}
				}
			}
		}
		else if (mySceneState == SceneState::Play)
		{
			Sandbox::Get().OnSceneStop();

			if (settings.sceneSettings.fullscreenOnPlay)
			{
				for (const auto& window : Sandbox::Get().GetEditorWindows())
				{
					if (window->GetTitle() == "Scene View" ||
						window->GetTitle() == "Asset Browser" ||
						window->GetTitle() == "Properties")
					{
						const_cast<bool&>(window->IsOpen()) = true;
					}
				}
			}
		}

	}

	ImGui::SameLine();

	Ref<Volt::Texture2D> physicsIcon = myAnimatedPhysicsIcon.GetCurrentFrame();
	static Volt::Texture2D* physicsId = physicsIcon.get();

	if (ImGui::ImageButtonAnimated(UI::GetTextureID(physicsId), UI::GetTextureID(physicsIcon), { size, size }, { 0.f, 0.f }, { 1.f, 1.f }, 0))
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
		localWorldIcon = EditorIconLibrary::GetIcon(EditorIcon::WorldSpace);
	}
	else
	{
		localWorldIcon = EditorIconLibrary::GetIcon(EditorIcon::LocalSpace);
	}

	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 2.f);

	if (ImGui::Button("FS"))
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

	if (UI::ImageButtonState("##snapToGrid", settings.sceneSettings.snapToGrid, UI::GetTextureID(EditorIconLibrary::GetIcon(EditorIcon::SnapGrid)), { buttonSize, buttonSize }))
	{
		settings.sceneSettings.snapToGrid = !settings.sceneSettings.snapToGrid;
	}

	ImGui::SetNextWindowSize({ 100.f, m_snapToGridValues.size() * 20.f });
	if (ImGui::BeginPopupContextItem("##gridSnapValues", ImGuiPopupFlags_MouseButtonRight))
	{
		for (uint32_t i = 0; i < m_snapToGridValues.size(); i++)
		{
			std::string valueStr = Utils::RemoveTrailingZeroes(std::to_string(m_snapToGridValues[i]));
			std::string	id = valueStr + "##gridSnapValue" + std::to_string(i);

			if (ImGui::Selectable(id.c_str()))
			{
				settings.sceneSettings.gridSnapValue = m_snapToGridValues[i];
			}
		}

		ImGui::EndPopup();
	}
	ImGui::SameLine();

	if (UI::ImageButtonState("##snapRotation", settings.sceneSettings.snapRotation, UI::GetTextureID(EditorIconLibrary::GetIcon(EditorIcon::SnapRotation)), { buttonSize, buttonSize }))
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

	if (UI::ImageButtonState("##snapScale", settings.sceneSettings.snapScale, UI::GetTextureID(EditorIconLibrary::GetIcon(EditorIcon::SnapScale)), { buttonSize, buttonSize }))
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

	if (UI::ImageButtonState("##showGizmos", settings.sceneSettings.showGizmos, UI::GetTextureID(EditorIconLibrary::GetIcon(EditorIcon::ShowGizmos)), { buttonSize, buttonSize }))
	{
		settings.sceneSettings.showGizmos = !settings.sceneSettings.showGizmos;
		Sandbox::Get().SetShouldRenderGizmos(settings.sceneSettings.showGizmos);
	}
	ImGui::PopStyleVar(3);
	ImGui::End();
}

void ViewportPanel::OnEvent(Volt::Event& e)
{
	Volt::EventDispatcher dispatcher(e);
	dispatcher.Dispatch<Volt::MouseMovedEvent>(VT_BIND_EVENT_FN(ViewportPanel::OnMouseMoved));
	dispatcher.Dispatch<Volt::KeyPressedEvent>(VT_BIND_EVENT_FN(ViewportPanel::OnKeyPressedEvent));
	dispatcher.Dispatch<Volt::MouseButtonPressedEvent>(VT_BIND_EVENT_FN(ViewportPanel::OnMousePressed));
	dispatcher.Dispatch<Volt::MouseButtonReleasedEvent>(VT_BIND_EVENT_FN(ViewportPanel::OnMouseReleased));

	myAnimatedPhysicsIcon.OnEvent(e);
}

bool ViewportPanel::OnMouseMoved(Volt::MouseMovedEvent& e)
{
	Volt::MouseMovedViewportEvent moved{ myViewportMouseCoords.x, myViewportMouseCoords.y };
	Volt::Application::Get().OnEvent(moved);
	return false;
}

bool ViewportPanel::OnMousePressed(Volt::MouseButtonPressedEvent& e)
{
	switch (e.GetMouseButton())
	{
		case VT_MOUSE_BUTTON_RIGHT:
			if (myIsHovered)
			{
				ImGui::SetWindowFocus("Viewport");
			}
			break;
	}

	return false;
}

bool ViewportPanel::OnKeyPressedEvent(Volt::KeyPressedEvent& e)
{
	if (!myIsHovered || Volt::Input::IsMouseButtonPressed(VT_MOUSE_BUTTON_RIGHT))
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
				if (myCurrentRenderPass == mySceneRenderer->GetAllFramebuffers().size() - 1)
				{
					myShowRenderTargets = false;
				}
				else
				{
					myCurrentRenderPass++;
				}
			}
		}
		;
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
	gem::vec2 perspectiveSize = myPerspectiveBounds[1] - myPerspectiveBounds[0];

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

			newEntity.GetComponent<Volt::TagComponent>().tag = Volt::AssetManager::Get().GetPathFromAssetHandle(handle).stem().string();
			myCreatedEntity = newEntity;

			break;
		}

		case Volt::AssetType::MeshSource:
		{
			const std::filesystem::path meshSourcePath = Volt::AssetManager::Get().GetPathFromAssetHandle(handle);
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

			newEntity.GetComponent<Volt::TagComponent>().tag = Volt::AssetManager::Get().GetPathFromAssetHandle(handle).stem().string();
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

			Wire::EntityId id = prefab->Instantiate(myEditorScene->GetRegistry());
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

	gem::vec3 dir = myEditorCameraController->GetCamera()->ScreenToWorldCoords(myViewportMouseCoords, myViewportSize);
	gem::vec3 targetPos = myEditorCameraController->GetCamera()->GetPosition();

	while (targetPos.y > 0.f)
	{
		if (targetPos.y < (targetPos + dir).y)
		{
			break;
		}

		targetPos += dir;
	}

	myCreatedEntity.SetPosition({ targetPos.x, 0.f, targetPos.z });
}

void ViewportPanel::Resize(const gem::vec2& viewportSize)
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

gem::vec2 ViewportPanel::GetViewportLocalPosition(const ImVec2& mousePos)
{
	auto [mx, my] = mousePos;
	mx -= myPerspectiveBounds[0].x;
	my -= myPerspectiveBounds[0].y;

	gem::vec2 perspectiveSize = myPerspectiveBounds[1] - myPerspectiveBounds[0];
	gem::vec2 result = { mx, my };

	return result;
}

gem::mat4 ViewportPanel::CalculateAverageTransform()
{
	gem::mat4 result;

	gem::vec3 avgTranslation;
	gem::vec3 avgRotation;
	gem::vec3 avgScale;

	for (const auto& ent : SelectionManager::GetSelectedEntities())
	{
		gem::vec3 p = 0.f, r = 0.f, s = 1.f;
		gem::mat4 transform = myEditorScene->GetWorldSpaceTransform(Volt::Entity{ ent, myEditorScene.get() });

		gem::decompose(transform, p, r, s);

		avgTranslation += p;
		avgRotation = r;
		avgScale += s;
	}

	avgTranslation /= (float)SelectionManager::GetSelectedCount();
	avgScale /= (float)SelectionManager::GetSelectedCount();

	return gem::translate(gem::mat4(1.f), avgTranslation) * gem::mat4_cast(gem::quat(avgRotation)) * gem::scale(gem::mat4(1.f), avgScale);
}
