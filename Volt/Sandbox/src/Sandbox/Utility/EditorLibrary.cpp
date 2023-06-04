#include "sbpch.h"
#include "EditorLibrary.h"

#include "Sandbox/Window/EditorWindow.h"

#include <Volt/Log/Log.h>

void EditorLibrary::Clear()
{
	s_editors.clear();
}

void EditorLibrary::Sort()
{
	std::sort(s_editors.begin(), s_editors.end(), [](const auto& lhs, const auto& rhs) 
	{
		return lhs.editorWindow->GetTitle() < rhs.editorWindow->GetTitle();
	});
}

bool EditorLibrary::OpenAsset(Ref<Volt::Asset> asset)
{
	if (!asset)
	{
		return false;
	}

	const Volt::AssetType type = asset->GetType();
	auto it = std::find_if(s_editors.begin(), s_editors.end(), [type](const auto& lhs) { return lhs.assetType == type; });
	if (it == s_editors.end())
	{
		return false;
	}

	it->editorWindow->Open();
	it->editorWindow->OpenAsset(asset);
	return true;
}

Ref<EditorWindow> EditorLibrary::Get(Volt::AssetType type)
{
	auto it = std::find_if(s_editors.begin(), s_editors.end(), [type](const auto& lhs) { return lhs.assetType == type; });
	if (it == s_editors.end())
	{
		VT_CORE_ERROR("Editor for asset not registered!");
		return nullptr;
	}

	return it->editorWindow;
}
