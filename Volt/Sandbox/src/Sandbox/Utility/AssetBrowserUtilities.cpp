#include "sbpch.h"
#include "AssetBrowserUtilities.h"

#include "Sandbox/Window/AssetBrowser/AssetItem.h"

#include <Volt/Asset/AssetManager.h>
#include <Volt/Asset/Mesh/Material.h>
#include <Volt/Asset/Mesh/SubMaterial.h>
#include <Volt/Asset/Mesh/MeshCompiler.h>
#include <Volt/Asset/Animation/Animation.h>
#include <Volt/Asset/Animation/Skeleton.h>
#include <Volt/Asset/Importers/MeshTypeImporter.h>

#include <Volt/Rendering/Shader/Shader.h>
#include <Volt/Utility/UIUtility.h>

namespace AssetBrowser
{
	const float AssetBrowserUtilities::GetBrowserItemPadding()
	{
		return 10.f;
	}

	const ImVec2 AssetBrowserUtilities::GetBrowserItemSize(const float thumbnailSize)
	{
		constexpr float itemHeightModifier = 40.f;
		const float itemPadding = GetBrowserItemPadding();

		return { thumbnailSize + itemPadding, thumbnailSize + itemHeightModifier + itemPadding };
	}

	const ImVec2 AssetBrowserUtilities::GetBrowserItemMinPos()
	{
		const ImVec2 cursorPos = ImGui::GetCursorPos();
		const ImVec2 windowPos = ImGui::GetWindowPos();
		const float scrollYOffset = ImGui::GetScrollY();

		return cursorPos + windowPos - ImVec2{ 0.f, scrollYOffset };
	}

	const ImVec4 AssetBrowserUtilities::GetBrowserItemHoveredColor()
	{
		return { 0.2f, 0.56f, 1.f, 1.f };
	}

	const ImVec4 AssetBrowserUtilities::GetBrowserItemClickedColor()
	{
		return { 0.3f, 0.6f, 1.f, 1.f };
	}

	const ImVec4 AssetBrowserUtilities::GetBrowserItemSelectedColor()
	{
		return { 0.f, 0.44f, 1.f, 1.f };
	}

	const ImVec4 AssetBrowserUtilities::GetBrowserItemDefaultColor()
	{
		return { 0.28f, 0.28f, 0.28f, 1.f };
	}

	const ImVec4 AssetBrowserUtilities::GetBackgroundColor(bool isHovered, bool isSelected)
	{
		ImVec4 bgColor = GetBrowserItemDefaultColor();

		if (isHovered && ImGui::IsMouseDown(ImGuiMouseButton_Left))
		{
			bgColor = GetBrowserItemClickedColor();
		}
		else if (isHovered)
		{
			bgColor = GetBrowserItemHoveredColor();
		}
		else if (isSelected)
		{
			bgColor = GetBrowserItemSelectedColor();
		}

		return bgColor;
	}

	bool AssetBrowserUtilities::RenderAssetTypePopup(AssetItem* item)
	{
		const auto& functions = GetPopupRenderFunctions();
		if (!functions.contains(item->type))
		{
			return false;
		}

		functions.at(item->type)(item);
		return true;
	}

	const std::unordered_map<Volt::AssetType, std::function<void(AssetItem*)>>& AssetBrowserUtilities::GetPopupRenderFunctions()
	{
		static std::unordered_map<Volt::AssetType, std::function<void(AssetItem*)>> renderFunctions;

		if (renderFunctions.empty())
		{
			renderFunctions[Volt::AssetType::Shader] = [](AssetItem* item)
			{
				if (ImGui::MenuItem("Recompile Shader"))
				{
					Ref<Volt::Shader> shader = Volt::AssetManager::GetAsset<Volt::Shader>(item->path);
					if (shader->Reload(true))
					{
						UI::Notify(NotificationType::Success, "Shader Compiled!", std::format("Shader {} compiled succesfully!", item->path.string()));
					}
					else
					{
						UI::Notify(NotificationType::Error, "Shader Compilation Failed", std::format("Shader {} failed to compile!", item->path.string()));
					}
				}
			};

			renderFunctions[Volt::AssetType::MeshSource] = [](AssetItem* item)
			{
				if (ImGui::MenuItem("Import"))
				{
					item->meshImportData = {};
					item->meshImportData.destination = item->path.parent_path().string() + "\\" + item->path.stem().string() + ".vtmesh";
					item->meshToImportData.handle = item->handle;
					item->meshToImportData.path = item->path;
					item->meshToImportData.type = Volt::AssetType::MeshSource;

					UI::OpenModal("Import Mesh##assetBrowser");
				}
			};

			renderFunctions[Volt::AssetType::Mesh] = [](AssetItem* item)
			{
				if (ImGui::MenuItem("Reimport"))
				{
					EditorUtils::ReimportSourceMesh(item->handle);
				}
			};
			
			renderFunctions[Volt::AssetType::Animation] = [](AssetItem* item)
			{
				if (ImGui::MenuItem("Reimport"))
				{
					EditorUtils::ReimportSourceMesh(item->handle);
				}
			};

			renderFunctions[Volt::AssetType::Skeleton] = [](AssetItem* item)
			{
				if (ImGui::MenuItem("Reimport"))
				{
					EditorUtils::ReimportSourceMesh(item->handle);
				}
			};
		}

		return renderFunctions;
	}
}