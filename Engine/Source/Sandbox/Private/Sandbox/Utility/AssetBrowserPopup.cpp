#include "sbpch.h"
#include "Utility/AssetBrowserPopup.h"

#include "Sandbox/Utility/EditorResources.h"
#include "Sandbox/Utility/EditorUtilities.h"
#include "Sandbox/Utility/Theme.h"

#include <AssetSystem/AssetManager.h>

#include <Volt/Utility/UIUtility.h>
#include <Volt/Utility/StringUtility.h>

#include <CoreUtilities/Math/Hash.h>

#include <imgui.h>

AssetBrowserPopup::AssetBrowserPopup(const std::string& id, AssetType wantedType, Volt::AssetHandle& handle)
	: myId(id), myWantedType(wantedType), myHandle(handle)
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

		UI::ScopedColor background{ ImGuiCol_ChildBg, EditorTheme::DarkGreyBackground };
		if (ImGui::BeginChild("##scrolling", ImGui::GetContentRegionAvail())) //#TODO_Ivar: Optimize!
		{
			Vector<Volt::AssetHandle> items = Volt::AssetManager::GetAllAssetsOfType(myWantedType);
			//items.erase(std::remove_if(items.begin(), items.end(), [](Volt::AssetHandle handle)
			//{
			//	return Volt::AssetManager::IsMemoryAsset(handle);
			//}));
			
			state = RenderView(items);
			ImGui::EndChild();
		}
		ImGui::EndPopup();

	}
	return state;
}

inline static size_t GetHashFromMetadata(const Volt::AssetMetadata& metadata)
{
	size_t hash = 0;
	hash = std::hash<std::string>()(metadata.filePath.stem().string());
	hash = Math::HashCombine(hash, std::hash<uint64_t>()(metadata.handle));

	return hash;
}

AssetBrowserPopup::State AssetBrowserPopup::RenderView(const Vector<Volt::AssetHandle>& items)
{
	VT_PROFILE_FUNCTION();

	State state = State::Open;
	Vector<std::pair<std::string, Volt::AssetHandle>> nameHandle;

	for (const auto& handle : items)
	{
		const auto& metadata = Volt::AssetManager::GetMetadataFromHandle(handle);
		if (!metadata.IsValid())
		{
			continue;
		}

		if (metadata.type->IsSourceType())
		{
			continue;
		}

		const std::string assetName = metadata.filePath.stem().string();
		nameHandle.emplace_back(assetName, metadata.handle);
	}

	Vector<std::string> searchNames{};
	for (const auto& [name, handle] : nameHandle)
	{
		if (std::find(searchNames.begin(), searchNames.end(), name) != searchNames.end())
		{
			continue;
		}

		searchNames.emplace_back(name);
	}

	if (!mySearchQuery.empty())
	{
		searchNames = UI::GetEntriesMatchingQuery(mySearchQuery, searchNames);
	}

	for (const auto& [name, handle] : nameHandle)
	{
		if (std::find(searchNames.begin(), searchNames.end(), name) == searchNames.end())
		{
			continue;
		}

		std::string assetId = name + "##" + std::to_string(handle);

		UI::RenderMatchingTextBackground(mySearchQuery, name, EditorTheme::MatchingTextBackground);
		if (ImGui::Selectable(assetId.c_str()))
		{
			myHandle = handle;
			state = State::Changed;
			ImGui::CloseCurrentPopup();
		}
	}

	return state;
}
