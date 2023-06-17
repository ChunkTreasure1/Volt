#include "sbpch.h"
#include "AssetPreview.h"

#include <Volt/Scene/Entity.h>
#include <Volt/Scene/Scene.h>

#include <Volt/Components/LightComponents.h>
#include <Volt/Components/Components.h>
#include <Volt/Asset/AssetManager.h>

#include <Volt/Rendering/SceneRenderer.h>
#include <Volt/Rendering/Texture/Texture2D.h>
#include <Volt/Rendering/Camera/Camera.h>

#include <Volt/Asset/AssetManager.h>

AssetPreview::AssetPreview(const std::filesystem::path& path)
{
	myCamera = CreateRef<Volt::Camera>(60.f, 1.f, 1.f, 100000.f);

	myScene = Volt::Scene::CreateDefaultScene("Asset Preview", false);

	auto entId = myScene->GetAllEntitiesWith<Volt::SkylightComponent>().front();

	Volt::Entity skylightEnt{ entId, myScene.get() };
	auto& skylightComp = skylightEnt.GetComponent<Volt::SkylightComponent>();
	skylightComp.environmentHandle = Volt::AssetManager::GetAsset<Volt::Texture2D>("Editor/Textures/HDRIs/studio_small_08_4k.hdr")->handle;
	skylightComp.lod = 2.f;

	Volt::SceneRendererSpecification spec{};
	spec.debugName = "Asset Preview";
	spec.initialResolution = { 128, 128 };
	spec.scene = myScene;

	mySceneRenderer = CreateRef<Volt::SceneRenderer>(spec);

	myEntity = myScene->CreateEntity();
	myEntity.AddComponent<Volt::MeshComponent>();

	myAssetHandle = Volt::AssetManager::Get().GetAssetHandleFromPath(path);
}

void AssetPreview::Render()
{
	const bool wasLoaded = Volt::AssetManager::Get().IsLoaded(myAssetHandle);

	myEntity.GetComponent<Volt::MeshComponent>().handle = myAssetHandle;

	Ref<Volt::Mesh> mesh = Volt::AssetManager::GetAsset<Volt::Mesh>(myAssetHandle);

	const glm::vec3 rotation = { glm::radians(30.f), glm::radians(135.f), 0.f };
	myCamera->SetRotation(rotation);

	const glm::vec3 position = mesh->GetBoundingSphere().center - myCamera->GetForward() * mesh->GetBoundingSphere().radius * 1.5f;

	myCamera->SetPosition(position);
	mySceneRenderer->OnRenderEditor(myCamera);

	if (!wasLoaded)
	{
		Volt::AssetManager::Get().Unload(myAssetHandle);
	}


	myIsRendered = true;
}

const Ref<Volt::Image2D> AssetPreview::GetPreview() const
{
	return mySceneRenderer->GetFinalImage();
}

