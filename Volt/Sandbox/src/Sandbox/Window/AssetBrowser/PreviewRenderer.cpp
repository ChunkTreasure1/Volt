#include "sbpch.h"
#include "PreviewRenderer.h"

#include "Sandbox/Window/AssetBrowser/AssetItem.h"

#include <Volt/Core/Graphics/GraphicsDevice.h>
#include <Volt/Core/Graphics/GraphicsContext.h>

#include <Volt/Asset/AssetManager.h>
#include <Volt/Asset/Mesh/Material.h>

#include <Volt/Scene/Scene.h>

#include <Volt/Rendering/SceneRenderer.h>
#include <Volt/Rendering/Texture/Image2D.h>

#include <Volt/Components/LightComponents.h>

PreviewRenderer::PreviewRenderer()
{
	myCamera = CreateRef<Volt::Camera>(60.f, 1.f, 1.f, 100000.f);
	myPreviewScene = Volt::Scene::CreateDefaultScene("Preview", false);

	// Set HDRI
	{
		auto skylightEntities = myPreviewScene->GetAllEntitiesWith<Volt::SkylightComponent>();

		Volt::Entity ent{ skylightEntities.front(), myPreviewScene.get() };
		ent.GetComponent<Volt::SkylightComponent>().environmentHandle = Volt::AssetManager::GetAssetHandleFromPath("Engine/Textures/HDRIs/defaultHDRI.hdr");
	}

	myEntity = myPreviewScene->CreateEntity();
	myEntity.AddComponent<Volt::MeshComponent>();

	Volt::SceneRendererSpecification spec{};
	spec.initialResolution = { 256, 256 };
	spec.scene = myPreviewScene;

	Volt::SceneRendererSettings settings{};
	settings.enableSkybox = false;

	myPreviewRenderer = CreateRef<Volt::SceneRenderer>(spec, settings);
}

PreviewRenderer::~PreviewRenderer()
{
	myPreviewRenderer = nullptr;
	myPreviewScene = nullptr;
	myCamera = nullptr;
}

void PreviewRenderer::RenderPreview(Weak<AssetBrowser::AssetItem> assetItem)
{
	if (assetItem.expired())
	{
		return;
	}

	auto itemPtr = assetItem.lock();
	const auto assetType = Volt::AssetManager::GetAssetTypeFromHandle(itemPtr->handle);
	switch (assetType)
	{
		case Volt::AssetType::Mesh:
			RenderMeshPreview(assetItem);
			break;

		case Volt::AssetType::Material:
			RenderMaterialPreview(assetItem);
			break;

		default:
			return;
	}

	Volt::GraphicsContext::GetDevice()->WaitForIdle();

	Volt::ImageSpecification spec{};
	spec = myPreviewRenderer->GetFinalImage()->GetSpecification();

	itemPtr->previewImage = Volt::Image2D::Create(spec);

	auto commandBuffer = Volt::GraphicsContext::GetDevice()->GetSingleUseCommandBuffer(true);
	itemPtr->previewImage->CopyFromImage(commandBuffer, myPreviewRenderer->GetFinalImage());
	Volt::GraphicsContext::GetDevice()->FlushSingleUseCommandBuffer(commandBuffer);
}

void PreviewRenderer::RenderMeshPreview(Weak<AssetBrowser::AssetItem> assetItem)
{
	auto itemPtr = assetItem.lock();

	Ref<Volt::Mesh> mesh = Volt::AssetManager::GetAsset<Volt::Mesh>(itemPtr->handle);
	if (!mesh || !mesh->IsValid())
	{
		return;
	}

	const bool wasLoaded = Volt::AssetManager::Get().IsLoaded(itemPtr->handle);
	myEntity.GetComponent<Volt::MeshComponent>().handle = itemPtr->handle;
	myEntity.GetComponent<Volt::MeshComponent>().overrideMaterial = Volt::Asset::Null();

	const glm::vec3 rotation = { glm::radians(30.f), glm::radians(135.f), 0.f };
	myCamera->SetRotation(rotation);

	const glm::vec3 position = mesh->GetBoundingSphere().center - myCamera->GetForward() * mesh->GetBoundingSphere().radius * 2.f;
	myCamera->SetPosition(position);

	myPreviewRenderer->OnRenderEditor(myCamera);

	if (wasLoaded)
	{
		Volt::AssetManager::Get().Unload(itemPtr->handle);
	}
}

void PreviewRenderer::RenderMaterialPreview(Weak<AssetBrowser::AssetItem> assetItem)
{
	auto itemPtr = assetItem.lock();

	Ref<Volt::Material> material = Volt::AssetManager::GetAsset<Volt::Material>(itemPtr->handle);
	if (!material || !material->IsValid())
	{
		return;
	}

	myEntity.GetComponent<Volt::MeshComponent>().handle = Volt::AssetManager::GetAsset<Volt::Mesh>("Engine/Meshes/Primitives/SM_Sphere.vtmesh")->handle;
	myEntity.GetComponent<Volt::MeshComponent>().overrideMaterial = material->handle;

	myCamera->SetPosition({ 0.f, 0.f, -150.f });
	myCamera->SetRotation(0.f);

	myPreviewRenderer->OnRenderEditor(myCamera);
}
