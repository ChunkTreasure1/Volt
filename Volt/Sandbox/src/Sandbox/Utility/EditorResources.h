#pragma once

#include <Volt/Asset/Asset.h>

namespace Volt
{
	class Texture2D;
}

enum class EditorIcon
{
	Directory,
	Reload,
	Back,
	Settings,
	Search,
	Play,
	Stop,
	GenericFile,
	Save,
	Open,
	Add,
	Filter,

	Locked,
	Unlocked,
	Visible,
	Hidden,

	EntityGizmo,
	LightGizmo,
	LocalSpace,
	WorldSpace,

	SnapRotation,
	SnapScale,
	SnapGrid,
	ShowGizmos,

	GetMaterial,
	SetMaterial,

	Close,
	Minimize,
	Maximize,
	Windowize,

	Volt
};

class EditorResources
{
public:
	static void Initialize();
	static void Shutdown();

	static Ref<Volt::Texture2D> GetAssetIcon(Volt::AssetType type);
	static Ref<Volt::Texture2D> GetEditorIcon(EditorIcon icon);

private:
	static Ref<Volt::Texture2D> TryLoadIcon(const std::filesystem::path& path);

	inline static std::unordered_map<Volt::AssetType, Ref<Volt::Texture2D>> myAssetIcons;
	inline static std::unordered_map<EditorIcon, Ref<Volt::Texture2D>> myEditorIcons;

	EditorResources() = delete;
};