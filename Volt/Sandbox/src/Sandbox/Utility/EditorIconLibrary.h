#pragma once

#include <Volt/Core/Base.h>

#include <unordered_map>

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

class EditorIconLibrary
{
public:
	static void Initialize();
	static void Shutdown();

	static Ref<Volt::Texture2D> GetIcon(EditorIcon icon);

private:
	static void TryLoadIcon(EditorIcon icon, const std::filesystem::path& path);

	EditorIconLibrary() = delete;

	inline static std::unordered_map<EditorIcon, Ref<Volt::Texture2D>> s_icons;
 };