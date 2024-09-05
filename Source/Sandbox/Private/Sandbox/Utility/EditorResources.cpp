#include "sbpch.h"
#include "Utility/EditorResources.h"

#include <Volt/Asset/Importers/TextureImporter.h>
#include <Volt/Asset/Importers/MeshTypeImporter.h>
#include <AssetSystem/AssetManager.h>

#include <Volt/Asset/Mesh/Mesh.h>

#include <Volt/Rendering/Renderer.h>
#include <Volt/Rendering/ShapeLibrary.h>
#include <Volt/Rendering/Texture/Texture2D.h>

void EditorResources::Initialize()
{
	// Asset icons
	{
		m_assetIcons[AssetTypes::Material] = TryLoadIcon("Editor/Textures/Icons/AssetIcons/icon_material.dds");
		m_assetIcons[AssetTypes::Mesh] = TryLoadIcon("Editor/Textures/Icons/AssetIcons/icon_mesh.dds");
		m_assetIcons[AssetTypes::MeshSource] = TryLoadIcon("Editor/Textures/Icons/AssetIcons/icon_meshSource.dds");
		m_assetIcons[AssetTypes::NavMesh] = TryLoadIcon("Editor/Textures/Icons/AssetIcons/icon_navmesh.dds");
		m_assetIcons[AssetTypes::Skeleton] = TryLoadIcon("Editor/Textures/Icons/AssetIcons/icon_skeleton.dds");
		m_assetIcons[AssetTypes::Animation] = TryLoadIcon("Editor/Textures/Icons/AssetIcons/icon_animation.dds");
		m_assetIcons[AssetTypes::AnimatedCharacter] = TryLoadIcon("Editor/Textures/Icons/AssetIcons/icon_animatedCharacter.dds");
		m_assetIcons[AssetTypes::Scene] = TryLoadIcon("Editor/Textures/Icons/AssetIcons/icon_scene.dds");
		m_assetIcons[AssetTypes::ParticlePreset] = TryLoadIcon("Editor/Textures/Icons/AssetIcons/icon_particlePreset.dds");
		m_assetIcons[AssetTypes::Prefab] = TryLoadIcon("Editor/Textures/Icons/AssetIcons/icon_prefab.dds");
		m_assetIcons[AssetTypes::MonoScript] = TryLoadIcon("Editor/Textures/Icons/AssetIcons/icon_monoscript.dds");
		m_assetIcons[AssetTypes::BehaviorGraph] = TryLoadIcon("Editor/Textures/Icons/AssetIcons/icon_behaviorTree.dds");
		m_assetIcons[AssetTypes::MotionWeave] = TryLoadIcon("Editor/Textures/Icons/AssetIcons/icon_motionWeaveDatabase.dds");
	}

	// Editor Icons
	{
		m_editorIcons[EditorIcon::Directory] = TryLoadIcon("Editor/Textures/Icons/icon_directory.dds");
		m_editorIcons[EditorIcon::Back] = TryLoadIcon("Editor/Textures/Icons/icon_back.dds");
		m_editorIcons[EditorIcon::Reload] = TryLoadIcon("Editor/Textures/Icons/icon_reload.dds");
		m_editorIcons[EditorIcon::Search] = TryLoadIcon("Editor/Textures/Icons/icon_search.dds");
		m_editorIcons[EditorIcon::Settings] = TryLoadIcon("Editor/Textures/Icons/icon_settings.dds");
		m_editorIcons[EditorIcon::Play] = TryLoadIcon("Editor/Textures/Icons/icon_play.dds");
		m_editorIcons[EditorIcon::Stop] = TryLoadIcon("Editor/Textures/Icons/icon_stop.dds");
		m_editorIcons[EditorIcon::GenericFile] = TryLoadIcon("Editor/Textures/Icons/icon_file.dds");
		m_editorIcons[EditorIcon::Save] = TryLoadIcon("Editor/Textures/Icons/icon_save.dds");
		m_editorIcons[EditorIcon::Open] = TryLoadIcon("Editor/Textures/Icons/icon_open.dds");
		m_editorIcons[EditorIcon::Add] = TryLoadIcon("Editor/Textures/Icons/icon_add.dds");
		m_editorIcons[EditorIcon::Filter] = TryLoadIcon("Editor/Textures/Icons/icon_filter.dds");

		m_editorIcons[EditorIcon::Unlocked] = TryLoadIcon("Editor/Textures/Icons/icon_unlocked.dds");
		m_editorIcons[EditorIcon::Locked] = TryLoadIcon("Editor/Textures/Icons/icon_locked.dds");

		m_editorIcons[EditorIcon::Hidden] = TryLoadIcon("Editor/Textures/Icons/icon_hidden.dds");
		m_editorIcons[EditorIcon::Visible] = TryLoadIcon("Editor/Textures/Icons/icon_visible.dds");

		m_editorIcons[EditorIcon::EntityGizmo] = TryLoadIcon("Editor/Textures/Icons/icon_entityGizmo.dds");
		//Volt::Renderer::AddTexture(myEditorIcons[EditorIcon::EntityGizmo]->GetImage());

		m_editorIcons[EditorIcon::LightGizmo] = TryLoadIcon("Editor/Textures/Icons/icon_lightGizmo.dds");
		//Volt::Renderer::AddTexture(myEditorIcons[EditorIcon::LightGizmo]->GetImage());

		m_editorIcons[EditorIcon::LocalSpace] = TryLoadIcon("Editor/Textures/Icons/icon_localSpace.dds");
		m_editorIcons[EditorIcon::WorldSpace] = TryLoadIcon("Editor/Textures/Icons/icon_worldSpace.dds");

		m_editorIcons[EditorIcon::SnapRotation] = TryLoadIcon("Editor/Textures/Icons/icon_snapRotation.dds");
		m_editorIcons[EditorIcon::SnapScale] = TryLoadIcon("Editor/Textures/Icons/icon_snapScale.dds");
		m_editorIcons[EditorIcon::SnapGrid] = TryLoadIcon("Editor/Textures/Icons/icon_snapToGrid.dds");
		m_editorIcons[EditorIcon::ShowGizmos] = TryLoadIcon("Editor/Textures/Icons/icon_showGizmo.dds");

		m_editorIcons[EditorIcon::FullscreenOnPlay] = TryLoadIcon("Editor/Textures/Icons/icon_fullscreenOnPlay.dds");

		m_editorIcons[EditorIcon::GetMaterial] = TryLoadIcon("Editor/Textures/Icons/icon_getMaterial.dds");
		m_editorIcons[EditorIcon::SetMaterial] = TryLoadIcon("Editor/Textures/Icons/icon_setMaterial.dds");

		m_editorIcons[EditorIcon::Close] = TryLoadIcon("Editor/Textures/Icons/icon_close.dds");
		m_editorIcons[EditorIcon::Minimize] = TryLoadIcon("Editor/Textures/Icons/icon_minimize.dds");
		m_editorIcons[EditorIcon::Maximize] = TryLoadIcon("Editor/Textures/Icons/icon_maximize.dds");
		m_editorIcons[EditorIcon::Windowize] = TryLoadIcon("Editor/Textures/Icons/icon_windowize.dds");

		m_editorIcons[EditorIcon::Paint] = TryLoadIcon("Editor/Textures/Icons/icon_paintBrush.dds");
		m_editorIcons[EditorIcon::Select] = TryLoadIcon("Editor/Textures/Icons/icon_click.dds");
		m_editorIcons[EditorIcon::Fill] = TryLoadIcon("Editor/Textures/Icons/icon_dentalFilling.dds");
		m_editorIcons[EditorIcon::Swap] = TryLoadIcon("Editor/Textures/Icons/icon_swap.dds");
		m_editorIcons[EditorIcon::Remove] = TryLoadIcon("Editor/Textures/Icons/icon_remove.dds");

		m_editorIcons[EditorIcon::GraphPinAnimationPose] = TryLoadIcon("Editor/Textures/Icons/icon_graph_pin_anim_pose.dds");
		m_editorIcons[EditorIcon::GraphPinAnimationPoseFilled] = TryLoadIcon("Editor/Textures/Icons/icon_graph_pin_anim_pose_filled.dds");

		m_editorIcons[EditorIcon::StateMachineAliasState] = TryLoadIcon("Editor/Textures/Icons/icon_statemachine_alias_state.dds");
		m_editorIcons[EditorIcon::StateMachineAnimationState] = TryLoadIcon("Editor/Textures/Icons/icon_statemachine_animation_state.dds");

		m_editorIcons[EditorIcon::Volt] = TryLoadIcon("Editor/Textures/Icons/icon_volt.dds");
	}

	// Meshes
	{
		m_editorMeshes[EditorMesh::Cube] = TryLoadMesh("Engine/Meshes/Primitives/SM_Cube.vtasset");
		m_editorMeshes[EditorMesh::Capsule] = TryLoadMesh("Engine/Meshes/Primitives/SM_Capsule.vtasset");
		m_editorMeshes[EditorMesh::Cone] = TryLoadMesh("Engine/Meshes/Primitives/SM_Cone.vtasset");
		m_editorMeshes[EditorMesh::Cylinder] = TryLoadMesh("Engine/Meshes/Primitives/SM_Cylinder.vtasset");
		m_editorMeshes[EditorMesh::Plane] = TryLoadMesh("Engine/Meshes/Primitives/SM_Plane.vtasset");
		m_editorMeshes[EditorMesh::Sphere] = TryLoadMesh("Engine/Meshes/Primitives/SM_Sphere.vtasset");
		//myEditorMeshes[EditorMesh::Arrow] = TryLoadMesh("Editor/Meshes/Arrow/3dpil.vtasset");
	}
}

void EditorResources::Shutdown()
{
	m_assetIcons.clear();
	m_editorIcons.clear();
	m_editorMeshes.clear();
}

Ref<Volt::Texture2D> EditorResources::GetAssetIcon(AssetType type)
{
	if (!m_assetIcons.contains(type))
	{
		return nullptr;
	}

	return m_assetIcons.at(type);
}

Ref<Volt::Texture2D> EditorResources::GetEditorIcon(EditorIcon icon)
{
	if (!m_editorIcons.contains(icon))
	{
		return Volt::Renderer::GetDefaultResources().whiteTexture;
	}

		return m_editorIcons.at(icon);
	}

Ref<Volt::Mesh> EditorResources::GetEditorMesh(EditorMesh mesh)
{
	if (!m_editorMeshes.contains(mesh))
	{
		return Volt::ShapeLibrary::GetCube();
	}

	return m_editorMeshes.at(mesh);
}

Ref<Volt::Texture2D> EditorResources::TryLoadIcon(const std::filesystem::path& path)
{
	Ref<Volt::Texture2D> texture = CreateRef<Volt::Texture2D>();
	Volt::TextureImporter::ImportTexture(path, *texture);

	if (!texture->IsValid())
	{
		texture = Volt::Renderer::GetDefaultResources().whiteTexture;
	}

	return texture;
}

Ref<Volt::Mesh> EditorResources::TryLoadMesh(const std::filesystem::path& path)
{
	Ref<Volt::Mesh> mesh = Volt::AssetManager::QueueAsset<Volt::Mesh>(Volt::AssetManager::GetAssetHandleFromFilePath(path));
	if (!mesh)
	{
		mesh = Volt::ShapeLibrary::GetCube();
	}

	return mesh;
}
