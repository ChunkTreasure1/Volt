#include "sbpch.h"
#include "MeshPreviewPanel.h"

#include "Sandbox/Camera/EditorCameraController.h"
#include "Sandbox/Utility/EditorUtilities.h"
#include "Sandbox/Utility/EditorResources.h"

#include <Volt/Asset/AssetManager.h>
#include <Volt/Asset/Mesh/Material.h>
#include <Volt/Asset/Mesh/SubMaterial.h>

#include <Volt/Utility/UIUtility.h>

#include <Volt/Scene/Scene.h>
#include <Volt/Rendering/Texture/Texture2D.h>
#include <Volt/Rendering/RenderPipeline/ShaderRegistry.h>
#include <Volt/RenderingNew/SceneRendererNew.h>

#include <Volt/Components/Components.h>
#include <Volt/Components/LightComponents.h>
#include <Volt/Asset/Mesh/MeshCompiler.h>

MeshPreviewPanel::MeshPreviewPanel()
	: EditorWindow("Mesh Preview", true)
{
	myCameraController = CreateRef<EditorCameraController>(60.f, 1.f, 100000.f);
	myScene = Volt::Scene::CreateDefaultScene("Mesh Preview", false);

	// Preview entity
	{
		auto entity = myScene->CreateEntity();
		Volt::MeshComponent& comp = entity.AddComponent<Volt::MeshComponent>(entity);
		comp.SetMesh(Volt::AssetManager::GetAssetHandleFromFilePath("Engine/Meshes/Primitives/SM_Cube.vtmesh"));
		myPreviewEntity = entity;
	}
}

void MeshPreviewPanel::UpdateContent()
{
	UpdateViewport();
	UpdateProperties();
	UpdateToolbar();
	UpdateMeshList();
}

void MeshPreviewPanel::OpenAsset(Ref<Volt::Asset> asset)
{
	if (asset && asset->IsValid() && asset->GetType() == Volt::AssetType::Mesh)
	{
		myPreviewEntity.GetComponent<Volt::MeshComponent>().SetMesh(asset->handle);
		myCurrentMesh = std::reinterpret_pointer_cast<Volt::Mesh>(asset);
		mySelectedSubMesh = -1;
	}
}

void MeshPreviewPanel::OnEvent(Volt::Event& e)
{
	if (!IsOpen()) { return; }

	Volt::EventDispatcher dispatcher(e);
	dispatcher.Dispatch<Volt::AppRenderEvent>(VT_BIND_EVENT_FN(MeshPreviewPanel::OnRenderEvent));

	myCameraController->OnEvent(e);
}

void MeshPreviewPanel::OnOpen()
{
	// Scene Renderer
	{
		Volt::SceneRendererSpecification spec{};
		spec.debugName = "Mesh Preview";
		spec.scene = myScene;

		//Volt::SceneRendererSettings settings{};
		//settings.enableGrid = true;
		//settings.enableOutline = true;

		mySceneRenderer = CreateRef<Volt::SceneRendererNew>(spec);
	}
}

void MeshPreviewPanel::OnClose()
{
	mySceneRenderer = nullptr;
}

bool MeshPreviewPanel::OnRenderEvent(Volt::AppRenderEvent& e)
{
	//mySceneRenderer->ClearOutlineCommands();

	//if (mySelectedSubMesh != -1)
	//{
	//	mySceneRenderer->SubmitOutlineMesh(myCurrentMesh, (uint32_t)mySelectedSubMesh, { 1.f });
	//}

	mySceneRenderer->OnRenderEditor(myCameraController->GetCamera());
	return false;
}

void MeshPreviewPanel::UpdateViewport()
{
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 0.f, 0.f });
	ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2{ 0.f, 0.f });
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0.f, 0.f });

	ImGui::Begin("Viewport##meshPreview");

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

void MeshPreviewPanel::UpdateProperties()
{
	ImGui::Begin("Properties##meshPreview");

	if (ImGui::BeginChild("meshProperties", { ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y / 2.f }))
	{
		ImGui::TextUnformatted("Mesh Properties");

		if (UI::BeginProperties("##meshPreview"))
		{
			if (myCurrentMesh)
			{
				Volt::AssetHandle handle = myCurrentMesh->GetMaterial()->handle;
				if (EditorUtils::Property("Material", handle, Volt::AssetType::Material))
				{
					myCurrentMesh->SetMaterial(Volt::AssetManager::GetAsset<Volt::Material>(handle));
				}
			}

			UI::EndProperties();
		}

		ImGui::EndChild();
	}

	if (myCurrentMesh)
	{
		if (ImGui::BeginChild("subMeshProperties", { ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y / 2.f }))
		{
			UI::Header("Sub Mesh");

			if (mySelectedSubMesh != -1)
			{
				std::vector<std::string> subMaterialNames;
				for (const auto& [index, subMat] : myCurrentMesh->GetMaterial()->GetSubMaterials())
				{
					subMaterialNames.emplace_back(subMat->GetName());
				}

				UI::PushID();
				if (UI::BeginProperties("subMeshProperties"))
				{
					auto currentMaterial = (int32_t)myCurrentMesh->GetSubMeshesMutable().at((uint32_t)mySelectedSubMesh).materialIndex;
					if (UI::ComboProperty("Sub Material", currentMaterial, subMaterialNames))
					{
						myCurrentMesh->GetSubMeshesMutable().at((uint32_t)mySelectedSubMesh).materialIndex = (uint32_t)currentMaterial;
					}

					UI::EndProperties();
				}
				UI::PopID();
			}

			ImGui::EndChild();
		}
	}

	ImGui::End();
}

void MeshPreviewPanel::UpdateToolbar()
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 2.f));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0.f, 0.f));
	UI::ScopedColor button(ImGuiCol_Button, { 0.f, 0.f, 0.f, 0.f });
	UI::ScopedColor hovered(ImGuiCol_ButtonHovered, { 0.3f, 0.305f, 0.31f, 0.5f });
	UI::ScopedColor active(ImGuiCol_ButtonActive, { 0.5f, 0.505f, 0.51f, 0.5f });

	ImGui::Begin("##toolbarMeshPreview", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

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
		const std::filesystem::path meshPath = FileSystem::OpenFileDialogue({ { "Mesh (*.vtmesh)", "vtmesh" } });
		if (!meshPath.empty() && FileSystem::Exists(meshPath))
		{
			myCurrentMesh = Volt::AssetManager::GetAsset<Volt::Mesh>(meshPath);
			myPreviewEntity.GetComponent<Volt::MeshComponent>().SetMesh(myCurrentMesh->handle);
			mySelectedSubMesh = -1;
		}
	}
	ImGui::PopStyleVar(2);
	ImGui::End();
}

void MeshPreviewPanel::UpdateMeshList()
{
	ImGui::Begin("Mesh List##meshPreview");

	if (!myCurrentMesh)
	{
		ImGui::End();
		return;
	}

	for (int32_t i = 0; const auto & subMesh : myCurrentMesh->GetSubMeshes())
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

void MeshPreviewPanel::SaveCurrentMesh()
{
	if (!myCurrentMesh)
	{
		return;
	}

	const auto& currentMeshMeta = Volt::AssetManager::GetMetadataFromHandle(myCurrentMesh->handle);
	if (!currentMeshMeta.IsValid())
	{
		return;
	}

	if (!FileSystem::IsWriteable(currentMeshMeta.filePath))
	{
		UI::Notify(NotificationType::Error, "Unable to save Mesh!", std::format("Unable to save mesh {0}! It is not writeable!", currentMeshMeta.filePath.string()));
		return;
	}

	if (!Volt::MeshCompiler::TryCompile(myCurrentMesh, currentMeshMeta.filePath, myCurrentMesh->GetMaterial()->handle))
	{
		UI::Notify(NotificationType::Error, "Unable to save Mesh!", std::format("Unable to save mesh {0}!", currentMeshMeta.filePath.string()));
	}
	else
	{
		UI::Notify(NotificationType::Success, "Saved Mesh!", std::format("Mesh {0} was saved successfully", currentMeshMeta.filePath.string()));
	}
}
