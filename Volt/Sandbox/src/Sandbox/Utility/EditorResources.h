#pragma once

#include <Volt/Asset/Asset.h>

namespace Volt
{
	class Texture2D;
	class Mesh;
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

	FullscreenOnPlay,

	GetMaterial,
	SetMaterial,

	Close,
	Minimize,
	Maximize,
	Windowize,

	Paint,
	Select,
	Fill,
	Swap,
	Remove,

	GraphPinAnimationPose,
	GraphPinAnimationPoseFilled,

	StateMachineAliasState,
	StateMachineAnimationState,


	Volt,
};

enum class EditorMesh
{
	Cube = 0,
	Capsule,
	Cone,
	Cylinder,
	Plane,
	Sphere,
	Arrow
};

class EditorResources
{
public:
	static void Initialize();
	static void Shutdown();

	static Ref<Volt::Texture2D> GetAssetIcon(Volt::AssetType type);
	static Ref<Volt::Texture2D> GetEditorIcon(EditorIcon icon);
	static Ref<Volt::Mesh> GetEditorMesh(EditorMesh mesh);

private:
	static Ref<Volt::Texture2D> TryLoadIcon(const std::filesystem::path& path);
	static Ref<Volt::Mesh> TryLoadMesh(const std::filesystem::path& path);

	inline static std::unordered_map<Volt::AssetType, Ref<Volt::Texture2D>> m_assetIcons;
	inline static std::unordered_map<EditorIcon, Ref<Volt::Texture2D>> m_editorIcons;
	inline static std::unordered_map<EditorMesh, Ref<Volt::Mesh>> m_editorMeshes;

	EditorResources() = delete;
};
