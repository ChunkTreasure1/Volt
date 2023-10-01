#include "sbpch.h"
#include "AssetPreview.h"

#include <Volt/Scene/Entity.h>
#include <Volt/Scene/Scene.h>

#include <Volt/Components/LightComponents.h>
#include <Volt/Components/RenderingComponents.h>
#include <Volt/Asset/AssetManager.h>

#include <Volt/RenderingNew/SceneRendererNew.h>
#include <Volt/Rendering/Texture/Texture2D.h>
#include <Volt/Rendering/Camera/Camera.h>

#include <Volt/Asset/AssetManager.h>

AssetPreview::AssetPreview(const std::filesystem::path& path)
{
	m_camera = CreateRef<Volt::Camera>(60.f, 1.f, 1.f, 100000.f);

	m_scene = Volt::Scene::CreateDefaultScene("Asset Preview", false);

	auto entId = m_scene->GetAllEntitiesWith<Volt::SkylightComponent>().front();

	Volt::Entity skylightEnt{ entId, m_scene.get() };
	auto& skylightComp = skylightEnt.GetComponent<Volt::SkylightComponent>();
	skylightComp.environmentHandle = Volt::AssetManager::GetAsset<Volt::Texture2D>("Editor/Textures/HDRIs/studio_small_08_4k.hdr")->handle;
	skylightComp.lod = 2.f;

	Volt::SceneRendererSpecification spec{};
	spec.debugName = "Asset Preview";
	spec.initialResolution = { 128, 128 };
	spec.scene = m_scene;

	m_sceneRenderer = CreateRef<Volt::SceneRendererNew>(spec);

	m_entity = m_scene->CreateEntity();
	m_entity.AddComponent<Volt::MeshComponent>();

	m_assetHandle = Volt::AssetManager::Get().GetAssetHandleFromFilePath(path);
}

void AssetPreview::Render()
{
	const bool wasLoaded = Volt::AssetManager::Get().IsLoaded(m_assetHandle);

	m_entity.GetComponent<Volt::MeshComponent>().handle = m_assetHandle;

	Ref<Volt::Mesh> mesh = Volt::AssetManager::GetAsset<Volt::Mesh>(m_assetHandle);

	const glm::vec3 rotation = { glm::radians(30.f), glm::radians(135.f), 0.f };
	m_camera->SetRotation(rotation);

	const glm::vec3 position = mesh->GetBoundingSphere().center - m_camera->GetForward() * mesh->GetBoundingSphere().radius * 1.5f;

	m_camera->SetPosition(position);
	m_sceneRenderer->OnRenderEditor(m_camera);

	if (!wasLoaded)
	{
		Volt::AssetManager::Get().Unload(m_assetHandle);
	}


	m_isRenderered = true;
}

const Ref<Volt::RHI::Image2D> AssetPreview::GetPreview() const
{
	return m_sceneRenderer->GetFinalImage();
}

