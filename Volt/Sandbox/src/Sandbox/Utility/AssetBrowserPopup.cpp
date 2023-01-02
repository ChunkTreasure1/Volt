#include "sbpch.h"
#include "AssetBrowserPopup.h"

#include "Sandbox/Utility/EditorResources.h"

#include <Volt/Utility/UIUtility.h>
#include <Volt/Utility/StringUtility.h>
#include <Volt/Asset/AssetManager.h>
#include <imgui.h>

AssetBrowserPopup::AssetBrowserPopup(const std::string& id, Volt::AssetType wantedType, Volt::AssetHandle& handle, std::function<void(Volt::AssetHandle& value)> callback /* = nullptr */)
	: myId(id), myWantedType(wantedType), myHandle(handle), myCallback(callback)
{}

AssetBrowserPopup::State AssetBrowserPopup::Update()
{
	State state = State::Closed;

	ImGui::SetNextWindowSize({ 250.f, 500.f });
	if (ImGui::BeginPopup(myId.c_str(), ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
	{
		{
			UI::ScopedColor childColor{ ImGuiCol_ChildBg, { 0.2f, 0.2f, 0.2f, 1.f } };
			UI::ScopedStyleFloat rounding(ImGuiStyleVar_ChildRounding, 2.f);

			constexpr float barHeight = 32.f;
			constexpr float searchBarSize = 22.f;

			ImGui::BeginChild("##searchBar", { 0.f, barHeight });
			{
				UI::ShiftCursor(5.f, 4.f);
				ImGui::Image(UI::GetTextureID(EditorResources::GetEditorIcon(EditorIcon::Search)), { searchBarSize, searchBarSize });

				ImGui::SameLine();

				ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - ImGui::GetStyle().WindowPadding.x);

				UI::PushId();

				if (UI::InputText("", mySearchQuery))
				{
					if (!mySearchQuery.empty())
					{
						myHasSearchQuery = true;
						Search(mySearchQuery);
					}
					else
					{
						myHasSearchQuery = false;
					}
				}
				UI::PopId();

				ImGui::PopItemWidth();
			}
			ImGui::EndChild();
		}

		if (ImGui::BeginChild("##scrolling", ImGui::GetContentRegionAvail()))
		{
			if (myHasSearchQuery)
			{
				state = RenderView(mySearchResults);
			}
			else
			{
				std::vector<std::filesystem::path> items;
				for (const auto& [path, handle] : Volt::AssetManager::Get().GetAssetRegistry())
				{
					const Volt::AssetType assetType = Volt::AssetManager::Get().GetAssetTypeFromPath(path);
					if (myWantedType == Volt::AssetType::None || assetType == myWantedType)
					{
						items.emplace_back(path);
					}
				}

				state = RenderView(items);
			}

			ImGui::EndChild();
		}
		ImGui::EndPopup();

	}
	return state;
}

void AssetBrowserPopup::Search(const std::string& query)
{
	mySearchResults.clear();

	for (const auto& [path, handle] : Volt::AssetManager::Get().GetAssetRegistry())
	{
		const Volt::AssetType assetType = Volt::AssetManager::Get().GetAssetTypeFromPath(path);
		if (myWantedType == Volt::AssetType::None || assetType == myWantedType)
		{
			const std::string lowerName = Utils::ToLower(path.stem().string());

			if (lowerName.find(Utils::ToLower(query)) != std::string::npos)
			{
				mySearchResults.emplace_back(path);
			}
		}
	}
}

AssetBrowserPopup::State AssetBrowserPopup::RenderView(const std::vector<std::filesystem::path>& items)
{
	State state = State::Open;

	for (const auto& item : items)
	{
		const Volt::AssetHandle assetHandle = Volt::AssetManager::Get().GetAssetHandleFromPath(item);

		std::string assetId = item.stem().string() + "###" + std::to_string(assetHandle);
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
