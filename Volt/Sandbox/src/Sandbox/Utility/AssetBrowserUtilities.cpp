#include "sbpch.h"
#include "AssetBrowserUtilities.h"

#include "Sandbox/Window/AssetBrowser/AssetItem.h"

#include <Volt/Asset/Prefab.h>
#include <Volt/Asset/AssetManager.h>
#include <Volt/Asset/Mesh/Material.h>
#include <Volt/Asset/Mesh/SubMaterial.h>
#include <Volt/Asset/Mesh/MeshCompiler.h>
#include <Volt/Asset/Animation/Animation.h>
#include <Volt/Asset/Animation/Skeleton.h>
#include <Volt/Asset/Importers/MeshTypeImporter.h>

#include <Volt/Rendering/Shader/Shader.h>
#include <Volt/Rendering/Renderer.h>

#include <Volt/Utility/UIUtility.h>
#include <Volt/Utility/MeshExporterUtilities.h>

namespace AssetBrowser
{
	const float AssetBrowserUtilities::GetBrowserItemPadding()
	{
		return 4.f;
	}

	const ImVec2 AssetBrowserUtilities::GetBrowserItemSize(const float thumbnailSize)
	{
		const float itemPadding = GetBrowserItemPadding();

		return { thumbnailSize + itemPadding, thumbnailSize + myItemHeightModifier + itemPadding };
	}

	const ImVec2 AssetBrowserUtilities::GetBrowserItemPos()
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

	void AssetBrowserUtilities::SetMeshExport(AssetItem* item)
	{
		switch (item->type)
		{
			case Volt::AssetType::Mesh:
			{
				meshesToExport.emplace_back(Volt::AssetManager::GetAsset<Volt::Mesh>(item->handle));
				break;
			}

			case Volt::AssetType::Prefab:
			{
				auto scene = CreateRef<Volt::Scene>("ExportMeshScene");

				Volt::AssetManager::GetAsset<Volt::Prefab>(item->handle)->Instantiate(scene.get());
				std::vector<Volt::Entity> meshEntities;

				for (const auto& ent : scene->GetAllEntitiesWith<Volt::MeshComponent>())
				{
					meshEntities.emplace_back(Volt::Entity(ent, scene.get()));
				}

				meshesToExport = Volt::MeshExporterUtilities::GetMeshes(meshEntities);
				break;
			}
		}
	}

	const std::unordered_map<Volt::AssetType, std::function<void(AssetItem*)>>& AssetBrowserUtilities::GetPopupRenderFunctions()
	{
		static std::unordered_map<Volt::AssetType, std::function<void(AssetItem*)>> renderFunctions;

		if (renderFunctions.empty())
		{
			renderFunctions[Volt::AssetType::ShaderDefinition] = [](AssetItem* item)
			{
				if (ImGui::MenuItem("Recompile Shader"))
				{
					Ref<Volt::Shader> shader = Volt::AssetManager::GetAsset<Volt::Shader>(item->path);
					if (shader->Reload(true))
					{
						Volt::Renderer::ReloadShader(shader);
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
				if (ImGui::MenuItem("Export Mesh"))
				{
					SetMeshExport(item);
					UI::OpenModal(std::format("Mesh Export##assetBrowser{0}", std::to_string(item->handle)));
				}

				if (ImGui::MenuItem("Reimport"))
				{
					EditorUtils::ReimportSourceMesh(item->handle);
				}
			};

			renderFunctions[Volt::AssetType::Animation] = [](AssetItem* item)
			{
				if (ImGui::MenuItem("Reimport"))
				{
					UI::OpenModal(std::format("Reimport Animation##assetBrowser{0}", std::to_string(item->handle)));

				}
			};

			renderFunctions[Volt::AssetType::Skeleton] = [](AssetItem* item)
			{
				if (ImGui::MenuItem("Reimport"))
				{
					EditorUtils::ReimportSourceMesh(item->handle);
				}
			};

			renderFunctions[Volt::AssetType::Prefab] = [](AssetItem* item)
			{
				if (ImGui::MenuItem("Export Meshes"))
				{
					SetMeshExport(item);
					UI::OpenModal(std::format("Mesh Export##assetBrowser{0}", std::to_string(item->handle)));
				}
			};
		}

		return renderFunctions;
	}
}
