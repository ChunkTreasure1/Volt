#include "sbpch.h"
#include "EditorIconLibrary.h"

#include <Volt/Asset/AssetManager.h>
#include <Volt/Rendering/Texture/Texture2D.h>
#include <Volt/Rendering/Renderer.h>

void EditorIconLibrary::Initialize()
{
	TryLoadIcon(EditorIcon::Directory, "Editor/Textures/Icons/icon_directory.dds");
	TryLoadIcon(EditorIcon::Back, "Editor/Textures/Icons/icon_back.dds");
	TryLoadIcon(EditorIcon::Reload, "Editor/Textures/Icons/icon_reload.dds");
	TryLoadIcon(EditorIcon::Search, "Editor/Textures/Icons/icon_search.dds");
	TryLoadIcon(EditorIcon::Settings, "Editor/Textures/Icons/icon_settings.dds");
	TryLoadIcon(EditorIcon::Play, "Editor/Textures/Icons/icon_play.dds");
	TryLoadIcon(EditorIcon::Stop, "Editor/Textures/Icons/icon_stop.dds");
	TryLoadIcon(EditorIcon::GenericFile, "Editor/Textures/Icons/icon_file.dds");
	TryLoadIcon(EditorIcon::Save, "Editor/Textures/Icons/icon_save.dds");
	TryLoadIcon(EditorIcon::Open, "Editor/Textures/Icons/icon_open.dds");
	TryLoadIcon(EditorIcon::Add, "Editor/Textures/Icons/icon_add.dds");
	TryLoadIcon(EditorIcon::Filter, "Editor/Textures/Icons/icon_filter.dds");

	TryLoadIcon(EditorIcon::Unlocked, "Editor/Textures/Icons/icon_unlocked.dds");
	TryLoadIcon(EditorIcon::Locked, "Editor/Textures/Icons/icon_locked.dds");

	TryLoadIcon(EditorIcon::Hidden, "Editor/Textures/Icons/icon_hidden.dds");
	TryLoadIcon(EditorIcon::Visible, "Editor/Textures/Icons/icon_visible.dds");

	TryLoadIcon(EditorIcon::EntityGizmo, "Editor/Textures/Icons/icon_entityGizmo.dds");
	TryLoadIcon(EditorIcon::LightGizmo, "Editor/Textures/Icons/icon_lightGizmo.dds");
	TryLoadIcon(EditorIcon::LocalSpace, "Editor/Textures/Icons/icon_localSpace.dds");
	TryLoadIcon(EditorIcon::WorldSpace, "Editor/Textures/Icons/icon_worldSpace.dds");

	TryLoadIcon(EditorIcon::SnapRotation, "Editor/Textures/Icons/icon_snapRotation.dds");
	TryLoadIcon(EditorIcon::SnapScale, "Editor/Textures/Icons/icon_snapScale.dds");
	TryLoadIcon(EditorIcon::SnapGrid, "Editor/Textures/Icons/icon_snapToGrid.dds");
	TryLoadIcon(EditorIcon::ShowGizmos, "Editor/Textures/Icons/icon_showGizmo.dds");

	TryLoadIcon(EditorIcon::GetMaterial, "Editor/Textures/Icons/icon_getMaterial.dds");
	TryLoadIcon(EditorIcon::SetMaterial, "Editor/Textures/Icons/icon_setMaterial.dds");

	TryLoadIcon(EditorIcon::Close, "Editor/Textures/Icons/icon_close.dds");
	TryLoadIcon(EditorIcon::Minimize, "Editor/Textures/Icons/icon_minimize.dds");
	TryLoadIcon(EditorIcon::Maximize, "Editor/Textures/Icons/icon_maximize.dds");
	TryLoadIcon(EditorIcon::Windowize, "Editor/Textures/Icons/icon_windowize.dds");
	TryLoadIcon(EditorIcon::Volt, "Editor/Textures/Icons/icon_volt.dds");
}

void EditorIconLibrary::Shutdown()
{
	s_icons.clear();
}

Ref<Volt::Texture2D> EditorIconLibrary::GetIcon(EditorIcon icon)
{
	if (s_icons.find(icon) != s_icons.end())
	{
		return s_icons[icon];
	}

	VT_CORE_ASSERT(false);
	return nullptr;
}

void EditorIconLibrary::TryLoadIcon(EditorIcon icon, const std::filesystem::path& path)
{
	Ref<Volt::Texture2D> texture = Volt::AssetManager::GetAsset<Volt::Texture2D>(path);
	if (!texture)
	{
		//texture = Volt::Renderer::GetDefaultData().whiteTexture;
	}

	s_icons[icon] = texture;
}
