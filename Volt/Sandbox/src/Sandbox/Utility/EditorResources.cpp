#include "sbpch.h"
#include "EditorResources.h"

#include <Volt/Asset/Importers/TextureImporter.h>
#include <Volt/Rendering/Renderer.h>

void EditorResources::Initialize()
{
	// Asset icons
	{
		myAssetIcons[Volt::AssetType::Material] = TryLoadIcon("Editor/Textures/Icons/AssetIcons/icon_material.dds");
		myAssetIcons[Volt::AssetType::Mesh] = TryLoadIcon("Editor/Textures/Icons/AssetIcons/icon_mesh.dds");
		myAssetIcons[Volt::AssetType::MeshSource] = TryLoadIcon("Editor/Textures/Icons/AssetIcons/icon_meshSource.dds");
		myAssetIcons[Volt::AssetType::Skeleton] = TryLoadIcon("Editor/Textures/Icons/AssetIcons/icon_skeleton.dds");
		myAssetIcons[Volt::AssetType::Animation] = TryLoadIcon("Editor/Textures/Icons/AssetIcons/icon_animation.dds");
		myAssetIcons[Volt::AssetType::AnimatedCharacter] = TryLoadIcon("Editor/Textures/Icons/AssetIcons/icon_animatedCharacter.dds");
		myAssetIcons[Volt::AssetType::Scene] = TryLoadIcon("Editor/Textures/Icons/AssetIcons/icon_scene.dds");
		myAssetIcons[Volt::AssetType::ParticlePreset] = TryLoadIcon("Editor/Textures/Icons/AssetIcons/icon_particlePreset.dds");
		myAssetIcons[Volt::AssetType::Prefab] = TryLoadIcon("Editor/Textures/Icons/AssetIcons/icon_prefab.dds");
	}

	// Editor Icons
	{
		myEditorIcons[EditorIcon::Directory] = TryLoadIcon("Editor/Textures/Icons/icon_directory.dds");
		myEditorIcons[EditorIcon::Back] = TryLoadIcon("Editor/Textures/Icons/icon_back.dds");
		myEditorIcons[EditorIcon::Reload] = TryLoadIcon("Editor/Textures/Icons/icon_reload.dds");
		myEditorIcons[EditorIcon::Search] = TryLoadIcon("Editor/Textures/Icons/icon_search.dds");
		myEditorIcons[EditorIcon::Settings] = TryLoadIcon("Editor/Textures/Icons/icon_settings.dds");
		myEditorIcons[EditorIcon::Play] = TryLoadIcon("Editor/Textures/Icons/icon_play.dds");
		myEditorIcons[EditorIcon::Stop] = TryLoadIcon("Editor/Textures/Icons/icon_stop.dds");
		myEditorIcons[EditorIcon::GenericFile] = TryLoadIcon("Editor/Textures/Icons/icon_file.dds");
		myEditorIcons[EditorIcon::Save] = TryLoadIcon("Editor/Textures/Icons/icon_save.dds");
		myEditorIcons[EditorIcon::Open] = TryLoadIcon("Editor/Textures/Icons/icon_open.dds");
		myEditorIcons[EditorIcon::Add] = TryLoadIcon("Editor/Textures/Icons/icon_add.dds");
		myEditorIcons[EditorIcon::Filter] = TryLoadIcon("Editor/Textures/Icons/icon_filter.dds");
		
		myEditorIcons[EditorIcon::Unlocked] = TryLoadIcon("Editor/Textures/Icons/icon_unlocked.dds");
		myEditorIcons[EditorIcon::Locked] = TryLoadIcon("Editor/Textures/Icons/icon_locked.dds");
		
		myEditorIcons[EditorIcon::Hidden] = TryLoadIcon("Editor/Textures/Icons/icon_hidden.dds");
		myEditorIcons[EditorIcon::Visible] = TryLoadIcon("Editor/Textures/Icons/icon_visible.dds");
		
		myEditorIcons[EditorIcon::EntityGizmo] = TryLoadIcon("Editor/Textures/Icons/icon_entityGizmo.dds");
		myEditorIcons[EditorIcon::LightGizmo] = TryLoadIcon("Editor/Textures/Icons/icon_lightGizmo.dds");
		myEditorIcons[EditorIcon::LocalSpace] = TryLoadIcon("Editor/Textures/Icons/icon_localSpace.dds");
		myEditorIcons[EditorIcon::WorldSpace] = TryLoadIcon("Editor/Textures/Icons/icon_worldSpace.dds");
		
		myEditorIcons[EditorIcon::SnapRotation] = TryLoadIcon("Editor/Textures/Icons/icon_snapRotation.dds");
		myEditorIcons[EditorIcon::SnapScale] = TryLoadIcon("Editor/Textures/Icons/icon_snapScale.dds");
		myEditorIcons[EditorIcon::SnapGrid] = TryLoadIcon("Editor/Textures/Icons/icon_snapToGrid.dds");
		myEditorIcons[EditorIcon::ShowGizmos] = TryLoadIcon("Editor/Textures/Icons/icon_showGizmo.dds");
		
		myEditorIcons[EditorIcon::GetMaterial] = TryLoadIcon("Editor/Textures/Icons/icon_getMaterial.dds");
		myEditorIcons[EditorIcon::SetMaterial] = TryLoadIcon("Editor/Textures/Icons/icon_setMaterial.dds");

		myEditorIcons[EditorIcon::Close] = TryLoadIcon("Editor/Textures/Icons/icon_close.dds");
		myEditorIcons[EditorIcon::Minimize] = TryLoadIcon("Editor/Textures/Icons/icon_minimize.dds");
		myEditorIcons[EditorIcon::Maximize] = TryLoadIcon("Editor/Textures/Icons/icon_maximize.dds");
		myEditorIcons[EditorIcon::Windowize] = TryLoadIcon("Editor/Textures/Icons/icon_windowize.dds");
		myEditorIcons[EditorIcon::Volt] = TryLoadIcon("Editor/Textures/Icons/icon_volt.dds");
	}
}

void EditorResources::Shutdown()
{
	myAssetIcons.clear();
	myEditorIcons.clear();
}

Ref<Volt::Texture2D> EditorResources::GetAssetIcon(Volt::AssetType type)
{
	if (!myAssetIcons.contains(type))
	{
		return nullptr;
	}

	return myAssetIcons.at(type);
}

Ref<Volt::Texture2D> EditorResources::GetEditorIcon(EditorIcon icon)
{
	if (!myEditorIcons.contains(icon))
	{
		return Volt::Renderer::GetDefaultData().whiteTexture;
	}

	return myEditorIcons.at(icon);
}

Ref<Volt::Texture2D> EditorResources::TryLoadIcon(const std::filesystem::path& path)
{
	Ref<Volt::Texture2D> texture = Volt::TextureImporter::ImportTexture(path);
	if (!texture)
	{
		texture = Volt::Renderer::GetDefaultData().whiteTexture;
	}

	return texture;
}
