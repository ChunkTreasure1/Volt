#include "sbpch.h"
#include "MeshPreviewPanel.h"

#include "Sandbox/Camera/EditorCameraController.h"
#include "Sandbox/Utility/EditorUtilities.h"
#include "Sandbox/Utility/EditorResources.h"

#include <Volt/Asset/AssetManager.h>
#include <Volt/Asset/Mesh/Material.h>

#include <Volt/Utility/UIUtility.h>

#include <Volt/Scene/Scene.h>
#include <Volt/Rendering/SceneRenderer.h>
#include <Volt/Rendering/Texture/Texture2D.h>
#include <Volt/Rendering/Renderer.h>
#include <Volt/Rendering/Shader/ShaderRegistry.h>
#include <Volt/Rendering/Framebuffer.h>

#include <Volt/Components/Components.h>
#include <Volt/Components/LightComponents.h>
#include <Volt/Components/PostProcessComponents.h>
#include <Volt/Asset/Mesh/MeshCompiler.h>

MeshPreviewPanel::MeshPreviewPanel()
	: EditorWindow("Mesh Preview", true)
{
	myGridMaterial = Volt::Material::Create(Volt::ShaderRegistry::Get("Grid"));
	myCameraController = CreateRef<EditorCameraController>(60.f, 1.f, 100000.f);
	myScene = CreateRef<Volt::Scene>();

	// Material sphere
	{
		auto entity = myScene->CreateEntity();
		Volt::MeshComponent& comp = entity.AddComponent<Volt::MeshComponent>();
		comp.handle = Volt::AssetManager::GetAsset<Volt::Mesh>("Assets/Meshes/Primitives/SM_Cube.vtmesh")->handle;
		myPreviewEntity = entity;
	}

	// Skylight
	{
		auto entity = myScene->CreateEntity();
		Volt::SkylightComponent& comp = entity.AddComponent<Volt::SkylightComponent>();
		comp.environmentHandle = Volt::AssetManager::GetAsset<Volt::Texture2D>("Assets/Textures/HDRIs/studio_small_08_4k.hdr")->handle;
	}

	// Directional light
	{
		auto entity = myScene->CreateEntity();
		Volt::DirectionalLightComponent& comp = entity.AddComponent<Volt::DirectionalLightComponent>();
		comp.castShadows = false;
		comp.intensity = 2.f;

		entity.SetLocalRotation(gem::quat(gem::radians(gem::vec3{ 70.f, 0.f, 100.f })));
	}
	
	{
		auto ent = myScene->CreateEntity();
		ent.GetComponent<Volt::TagComponent>().tag = "Post Processing";
		ent.AddComponent<Volt::BloomComponent>();
		ent.AddComponent<Volt::FXAAComponent>();
		ent.AddComponent<Volt::HBAOComponent>();
	}

	mySceneRenderer = CreateRef<Volt::SceneRenderer>(myScene);
	mySceneRenderer->Resize(1280, 720);

	//mySceneRenderer->AddForwardCallback([this](Ref<Volt::Scene> scene, Ref<Volt::Camera> camera)
	//	{
	//		Volt::Renderer::SubmitSprite(gem::mat4{ 1.f }, { 1.f, 1.f, 1.f, 1.f }, myGridMaterial);
	//		Volt::Renderer::DispatchSpritesWithMaterial(myGridMaterial);
	//	});
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
		myPreviewEntity.GetComponent<Volt::MeshComponent>().handle = asset->handle;
		myCurrentMesh = std::reinterpret_pointer_cast<Volt::Mesh>(asset);
		mySelectedSubMesh = 0;
	}
}

void MeshPreviewPanel::OnEvent(Volt::Event& e)
{
	Volt::EventDispatcher dispatcher(e);
	dispatcher.Dispatch<Volt::AppRenderEvent>(VT_BIND_EVENT_FN(MeshPreviewPanel::OnRenderEvent));

	myCameraController->OnEvent(e);
}

bool MeshPreviewPanel::OnRenderEvent(Volt::AppRenderEvent& e)
{
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
	if (myViewportSize != (*(gem::vec2*)&viewportSize) && viewportSize.x > 0 && viewportSize.y > 0 && !Volt::Input::IsMouseButtonPressed(VT_MOUSE_BUTTON_LEFT))
	{
		myViewportSize = { viewportSize.x, viewportSize.y };
		mySceneRenderer->Resize((uint32_t)myViewportSize.x, (uint32_t)myViewportSize.y);
		myCameraController->UpdateProjection((uint32_t)myViewportSize.x, (uint32_t)myViewportSize.y);
	}

	ImGui::Image(UI::GetTextureID(mySceneRenderer->GetFinalFramebuffer()->GetColorAttachment(0)), viewportSize);
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
			ImGui::TextUnformatted("Sub Mesh Properties");

			if (ImGui::Button("Flip V"))
			{
				FlipV();
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
		const std::filesystem::path characterPath = FileSystem::OpenFile("Animated Character (*.vtchr)\0*.vtchr\0");
		if (!characterPath.empty() && FileSystem::Exists(characterPath))
		{
			myCurrentMesh = Volt::AssetManager::GetAsset<Volt::Mesh>(characterPath);
			myPreviewEntity.GetComponent<Volt::MeshComponent>().handle = myCurrentMesh->handle;
			mySelectedSubMesh = 0;
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

void MeshPreviewPanel::SaveCurrentMesh()
{
	if (!myCurrentMesh)
	{
		return;
	}

	if (!FileSystem::IsWriteable(myCurrentMesh->path))
	{
		UI::Notify(NotificationType::Error, "Unable to save Mesh!", std::format("Unable to save mesh {0}! It is not writeable!", myCurrentMesh->path.string()));
		return;
	}

	if (!Volt::MeshCompiler::TryCompile(myCurrentMesh, myCurrentMesh->path, myCurrentMesh->GetMaterial()->handle))
	{
		UI::Notify(NotificationType::Error, "Unable to save Mesh!", std::format("Unable to save mesh {0}!", myCurrentMesh->path.string()));
	}
	else
	{
		UI::Notify(NotificationType::Success, "Saved Mesh!", std::format("Mesh {0} was saved successfully", myCurrentMesh->path.string()));
	}
}

void MeshPreviewPanel::FlipV()
{
	auto& vertices = const_cast<std::vector<Volt::Vertex>&>(myCurrentMesh->GetVertices());
	const auto& subMesh = myCurrentMesh->GetSubMeshes().at(mySelectedSubMesh);

	for (uint32_t i = subMesh.vertexStartOffset; i < subMesh.vertexCount; i++)
	{
		vertices[i].texCoords[0].y = 1.f - vertices[i].texCoords[0].y;
		vertices[i].texCoords[1].y = 1.f - vertices[i].texCoords[1].y;
		vertices[i].texCoords[2].y = 1.f - vertices[i].texCoords[2].y;
		vertices[i].texCoords[3].y = 1.f - vertices[i].texCoords[3].y;
	}

	myCurrentMesh->Construct();
}
