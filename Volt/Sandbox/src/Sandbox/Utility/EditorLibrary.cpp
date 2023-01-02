#include "sbpch.h"
#include "EditorLibrary.h"

#include "Sandbox/Window/EditorWindow.h"

#include <Volt/Log/Log.h>

void EditorLibrary::Clear()
{
	s_editors.clear();
}

void EditorLibrary::Register(Volt::AssetType type, Ref<EditorWindow> editor)
{
	s_editors.emplace(type, editor);
}

void EditorLibrary::OpenAsset(Ref<Volt::Asset> asset)
{
	if (!asset)
	{
		return;
	}

	const Volt::AssetType type = asset->GetType();
	auto it = s_editors.find(type);
	if (it == s_editors.end())
	{
		VT_CORE_ERROR("Editor for asset not registered!");
		return;
	}

	it->second->Open();
	it->second->OpenAsset(asset);
}

Ref<EditorWindow> EditorLibrary::Get(Volt::AssetType type)
{
	auto it = s_editors.find(type);
	if (it == s_editors.end())
	{
		VT_CORE_ERROR("Editor for asset not registered!");
		return nullptr;
	}

	return it->second;
}
