#include "sbpch.h"
#include "PrefabEditorPanel.h"

#include "Sandbox/Camera/EditorCameraController.h"
#include "Sandbox/Utility/EditorUtilities.h"
#include "Sandbox/Utility/EditorResources.h"

#include <Volt/Asset/AssetManager.h>
#include <Volt/Asset/Mesh/Mesh.h>

#include <Volt/Utility/UIUtility.h>

#include <Volt/Scene/Scene.h>
#include <Volt/Rendering/Texture/Texture2D.h>

#include <Volt/Rendering/SceneRenderer.h>

#include <Volt/Components/CoreComponents.h>
#include <Volt/Components/RenderingComponents.h>
#include <Volt/Components/LightComponents.h>
#include <Volt/Asset/Mesh/MeshCompiler.h>

PrefabEditorPanel::PrefabEditorPanel()
	: EditorWindow("Prefab Editor", true)
{
	myCameraController = CreateRef<EditorCameraController>(60.f, 1.f, 100000.f);
	myScene = Volt::Scene::CreateDefaultScene("Prefab Editor", false);

	myScene->Clear();

	mySceneViewPanel = CreateRef<SceneViewPanel>(myScene, "##PrefabEditor");
	myPropertiesPanel = CreateRef<PropertiesPanel>(myScene, mySceneRenderer, mySceneState, "##PrefabEditor");
}

void PrefabEditorPanel::UpdateMainContent()
{
	UpdateViewport();
	UpdateSceneView();
	UpdateProperties();

	//UpdateToolbar();
}

void PrefabEditorPanel::OpenAsset(Ref<Volt::Asset> asset)
{
	if (asset && asset->IsValid() && asset->GetType() == Volt::AssetType::Mesh)
	{
		myPreviewEntity.GetComponent<Volt::MeshComponent>().handle = asset->handle;
		myCurrentMesh = std::reinterpret_pointer_cast<Volt::Mesh>(asset);
		mySelectedSubMesh = 0;
	}
}

void PrefabEditorPanel::OnEvent(Volt::Event& e)
{
	Volt::EventDispatcher dispatcher(e);
	dispatcher.Dispatch<Volt::AppRenderEvent>(VT_BIND_EVENT_FN(PrefabEditorPanel::OnRenderEvent));

	myCameraController->OnEvent(e);
}

void PrefabEditorPanel::OnOpen()
{
	// Scene Renderer
	{
		Volt::SceneRendererSpecification spec{};
		spec.debugName = "Prefab Editor";
		spec.scene = myScene;

		//Volt::SceneRendererSettings settings{};
		//settings.enableGrid = true;

		mySceneRenderer = CreateRef<Volt::SceneRenderer>(spec);
	}
}

void PrefabEditorPanel::OnClose()
{
	mySceneRenderer = nullptr;
}

bool PrefabEditorPanel::OnRenderEvent(Volt::AppRenderEvent& e)
{
	mySceneRenderer->OnRenderEditor(myCameraController->GetCamera());
	return false;
}

void PrefabEditorPanel::UpdateViewport()
{
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 0.f, 0.f });
	ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2{ 0.f, 0.f });
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0.f, 0.f });

	ImGui::Begin("Viewport##prefabEditor");

	myCameraController->SetIsControllable(ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows));

	auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
	auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
	auto viewportOffset = ImGui::GetWindowPos();

	myPerspectiveBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
	myPerspectiveBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

	ImVec2 viewportSize = ImGui::GetContentRegionAvail();
	if (myViewportSize != (*(glm::vec2*)&viewportSize) && viewportSize.x > 0 && viewportSize.y > 0 && !Volt::Input::IsMouseButtonDown(VT_MOUSE_BUTTON_LEFT))
	{
		myViewportSize = { viewportSize.x, viewportSize.y };
		mySceneRenderer->Resize((uint32_t)myViewportSize.x, (uint32_t)myViewportSize.y);
		myCameraController->UpdateProjection((uint32_t)myViewportSize.x, (uint32_t)myViewportSize.y);
	}

	ImGui::Image(UI::GetTextureID(mySceneRenderer->GetFinalImage()), viewportSize);
	ImGui::End();
	ImGui::PopStyleVar(3);
}

void PrefabEditorPanel::UpdateSceneView()
{
	if (mySceneViewPanel->Begin())
	{
		mySceneViewPanel->UpdateMainContent();
		mySceneViewPanel->End();
		mySceneViewPanel->UpdateContent();
	}
}

void PrefabEditorPanel::UpdateProperties()
{
	if (myPropertiesPanel->Begin())
	{
		myPropertiesPanel->UpdateMainContent();
		myPropertiesPanel->End();
		myPropertiesPanel->UpdateContent();
	}
}

void PrefabEditorPanel::UpdateToolbar()
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 2.f));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0.f, 0.f));
	UI::ScopedColor button(ImGuiCol_Button, { 0.f, 0.f, 0.f, 0.f });
	UI::ScopedColor hovered(ImGuiCol_ButtonHovered, { 0.3f, 0.305f, 0.31f, 0.5f });
	UI::ScopedColor active(ImGuiCol_ButtonActive, { 0.5f, 0.505f, 0.51f, 0.5f });

	ImGui::Begin("##toolbarPrefabEditor", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

	if (UI::ImageButton("##Save", UI::GetTextureID(EditorResources::GetEditorIcon(EditorIcon::Save)), { myButtonSize, myButtonSize }))
	{
		if (myCurrentMesh)
		{
			SaveCurrentMesh();
		}
	}

	ImGui::SameLine();

	if (UI::ImageButton("##Load", UI::GetTextureID(EditorResources::GetEditorIcon(EditorIcon::Open)), { myButtonSize, myButtonSize }))
	{
		const std::filesystem::path prefabPath = FileSystem::OpenFileDialogue({{ "Prefab (*.vtprefab)", "vtchr" }});
		if (!prefabPath.empty() && FileSystem::Exists(prefabPath))
		{
			myCurrentMesh = Volt::AssetManager::GetAsset<Volt::Mesh>(prefabPath);
			myPreviewEntity.GetComponent<Volt::MeshComponent>().handle = myCurrentMesh->handle;
			mySelectedSubMesh = 0;
		}
	}
	ImGui::PopStyleVar(2);
	ImGui::End();
}

void PrefabEditorPanel::UpdateMeshList()
{
	ImGui::Begin("Mesh List##prefabEditor");

	if (!myCurrentMesh)
	{
		ImGui::End();
		return;
	}

	for (uint32_t i = 0; const auto & subMesh : myCurrentMesh->GetSubMeshes())
	{
		std::string id = subMesh.name + "##subMesh" + std::to_string(i);

		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
		if (mySelectedSubMesh == i)
		{
			flags |= ImGuiTreeNodeFlags_Selected;
		}

		ImGui::TreeNodeEx(id.c_str(), flags);
		if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
		{
			mySelectedSubMesh = i;
		}

		i++;
	}

	ImGui::End();
}

void PrefabEditorPanel::SaveCurrentMesh()
{
	if (!myCurrentMesh)
	{
		return;
	}

	const auto filesystemPath = Volt::AssetManager::GetFilesystemPath(myCurrentMesh->handle);
	const auto& metadata = Volt::AssetManager::GetMetadataFromHandle(myCurrentMesh->handle);

	if (!FileSystem::IsWriteable(filesystemPath))
	{
		UI::Notify(NotificationType::Error, "Unable to save Mesh!", std::format("Unable to save mesh {0}! It is not writeable!", metadata.filePath.string()));
		return;
	}

	if (!Volt::MeshCompiler::TryCompile(myCurrentMesh, metadata.filePath, myCurrentMesh->GetMaterialTable()))
	{
		UI::Notify(NotificationType::Error, "Unable to save Mesh!", std::format("Unable to save mesh {0}!", metadata.filePath.string()));
	}
	else
	{
		UI::Notify(NotificationType::Success, "Saved Mesh!", std::format("Mesh {0} was saved successfully", metadata.filePath.string()));
	}
}
