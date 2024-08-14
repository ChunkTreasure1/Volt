#include "sbpch.h"
#include "EditorLibrary.h"

#include "Sandbox/Window/EditorWindow.h"

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

Ref<EditorWindow> EditorLibrary::GetPanel(const std::string& panelName)
{
	for (const auto& panel : s_editors)
	{
		if (panel.editorWindow->GetTitle() == panelName)
		{
			return panel.editorWindow;
		}
	}

	return nullptr;
}

bool EditorLibrary::OpenAsset(Ref<Volt::Asset> asset)
{
	if (!asset)
	{
		return false;
	}

	const AssetType type = asset->GetType();
	auto it = std::find_if(s_editors.begin(), s_editors.end(), [type](const auto& lhs) { return lhs.assetType == type; });
	if (it == s_editors.end())
	{
		return false;
	}

	it->editorWindow->Open();
	it->editorWindow->OpenAsset(asset);
	return true;
}

Ref<EditorWindow> EditorLibrary::Get(AssetType type)
{
	auto it = std::find_if(s_editors.begin(), s_editors.end(), [type](const auto& lhs) { return lhs.assetType == type; });
	if (it == s_editors.end())
	{
		VT_LOG(Error, "Editor for asset not registered!");
		return nullptr;
	}

	return it->editorWindow;
}
