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
#include <Volt/Rendering/Framebuffer.h>

#include <Volt/Components/PostProcessComponents.h>

#include <Volt/Asset/AssetManager.h>

AssetPreview::AssetPreview(const std::filesystem::path& path)
{
	myCamera = CreateRef<Volt::Camera>(60.f, 1.f, 1.f, 100000.f);

    myScene = CreateRef<Volt::Scene>();
    mySceneRenderer = CreateRef<Volt::SceneRenderer>(myScene, "Asset Preview", true);
    mySceneRenderer->Resize(256, 256);

    myEntity = myScene->CreateEntity();
    myEntity.AddComponent<Volt::MeshComponent>();

    myAssetHandle = Volt::AssetManager::Get().GetAssetHandleFromPath(path);

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
		comp.intensity = 3.f;

		entity.SetLocalRotation(gem::quat(gem::radians(gem::vec3{ 70.f, 0.f, 100.f })));
	}

	{
		auto ent = myScene->CreateEntity();
		ent.GetComponent<Volt::TagComponent>().tag = "Post Processing";
		ent.AddComponent<Volt::BloomComponent>();
		ent.AddComponent<Volt::FXAAComponent>();
		ent.AddComponent<Volt::HBAOComponent>();
	}
}

void AssetPreview::Render()
{
    const bool wasLoaded = Volt::AssetManager::Get().IsLoaded(myAssetHandle);

    myEntity.GetComponent<Volt::MeshComponent>().handle = myAssetHandle;

	Ref<Volt::Mesh> mesh = Volt::AssetManager::GetAsset<Volt::Mesh>(myAssetHandle);

	const gem::vec3 rotation = { gem::radians(30.f), gem::radians(135.f), 0.f };
	myCamera->SetRotation(rotation);

	const gem::vec3 position = gem::vec3{ 0.f, 0.f, 0.f } - myCamera->GetForward() * mesh->GetBoundingSphere().radius * 2.5f;

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
    return mySceneRenderer->GetFinalFramebuffer()->GetColorAttachment(0);
}
