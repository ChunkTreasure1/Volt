#include "sbpch.h"
#include "AssetBrowserPopup.h"

#include "Sandbox/Utility/EditorResources.h"
#include "Sandbox/Utility/EditorUtilities.h"
#include "Sandbox/Utility/Theme.h"

#include <Volt/Utility/UIUtility.h>
#include <Volt/Utility/StringUtility.h>
#include <Volt/Asset/AssetManager.h>
#include <imgui.h>

AssetBrowserPopup::AssetBrowserPopup(const std::string& id, Volt::AssetType wantedType, Volt::AssetHandle& handle, std::function<void(Volt::AssetHandle& value)> callback /* = nullptr */)
	: myId(id), myWantedType(wantedType), myHandle(handle), myCallback(callback)
{
}

AssetBrowserPopup::State AssetBrowserPopup::Update()
{
	State state = State::Closed;

	ImGui::SetNextWindowSize({ 250.f, 500.f });
	if (ImGui::BeginPopup(myId.c_str(), ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
	{
		// Search bar
		{
			bool t;
			EditorUtils::SearchBar(mySearchQuery, t, myActivateSearch);
			if (myActivateSearch)
			{
				myActivateSearch = false;
			}
		}

		UI::ScopedColor background{ ImGuiCol_ChildBg, EditorTheme::DarkBackground };
		if (ImGui::BeginChild("##scrolling", ImGui::GetContentRegionAvail())) //#TODO_Ivar: Optimize!
		{
			const std::vector<Volt::AssetHandle> items = Volt::AssetManager::GetAllAssetsOfType(myWantedType);
			state = RenderView(items);
			ImGui::EndChild();
		}
		ImGui::EndPopup();

	}
	return state;
}

AssetBrowserPopup::State AssetBrowserPopup::RenderView(const std::vector<Volt::AssetHandle>& items)
{
	State state = State::Open;
	std::vector<std::string> names;
	std::unordered_map<std::string, Volt::AssetHandle> nameToAssetHandleMap;

	for (const auto& handle : items)
	{
		const auto& metadata = Volt::AssetManager::GetMetadataFromHandle(handle);
		if (!metadata.IsValid())
		{
			continue;
		}

		names.emplace_back(metadata.filePath.stem().string());
		nameToAssetHandleMap[names.back()] = handle;
	}

	if (!mySearchQuery.empty())
	{
		names = UI::GetEntriesMatchingQuery(mySearchQuery, names);
	}

	for (const auto& name : names)
	{
		const Volt::AssetHandle assetHandle = nameToAssetHandleMap.at(name);
		std::string assetId = name + "###" + std::to_string(assetHandle);

		UI::RenderMatchingTextBackground(mySearchQuery, name, EditorTheme::MatchingTextBackground);
		if (ImGui::Selectable(assetId.c_str()))
		{
			myHandle = assetHandle;
			state = State::Changed;
			if (myCallback)
			{
				myCallback(myHandle);
			}

			ImGui::CloseCurrentPopup();
		}
	}

	return state;
}
