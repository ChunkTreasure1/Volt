#include "sbpch.h"
#include "Utility/AssetBrowserUtilities.h"

#include "Sandbox/Window/AssetBrowser/AssetItem.h"
#include "Sandbox/UISystems/ModalSystem.h"
#include "Sandbox/Modals/MeshImportModal.h"
#include "Sandbox/Modals/TextureImportModal.h"
#include "Sandbox/Sandbox.h"

#include <Volt/Asset/Prefab.h>
#include <AssetSystem/AssetManager.h>
#include <Volt/Asset/Mesh/MeshCompiler.h>
#include <Volt/Asset/Animation/Animation.h>
#include <Volt/Asset/Animation/Skeleton.h>
#include <Volt/Asset/TextureSource.h>

#include <Volt/Components/RenderingComponents.h>

#include <Volt/Utility/UIUtility.h>
#include <Volt/Utility/MeshExporterUtilities.h>

#include <Volt/Rendering/Texture/Texture2D.h>

#include <RHIModule/Images/Image.h>

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
		if (item->type == AssetTypes::Mesh)
		{
			meshesToExport.emplace_back(Volt::AssetManager::GetAsset<Volt::Mesh>(item->handle));
		}
		else if (item->type == AssetTypes::Prefab)
		{
			auto scene = CreateRef<Volt::Scene>("ExportMeshScene");

			Volt::AssetManager::GetAsset<Volt::Prefab>(item->handle)->Instantiate(scene);
			Vector<Volt::Entity> meshEntities;

			for (const auto& ent : scene->GetAllEntitiesWith<Volt::MeshComponent>())
			{
				meshEntities.emplace_back(ent);
			}

			meshesToExport = Volt::MeshExporterUtilities::GetMeshes(meshEntities);
		}
	}

	const std::unordered_map<AssetType, std::function<void(AssetItem*)>>& AssetBrowserUtilities::GetPopupRenderFunctions()
	{
		static std::unordered_map<AssetType, std::function<void(AssetItem*)>> renderFunctions;

		if (renderFunctions.empty())
		{
			renderFunctions[AssetTypes::ShaderDefinition] = [](AssetItem* item)
			{
				if (ImGui::MenuItem("Recompile Shader"))
				{
					/*Ref<Volt::Shader> shader = Volt::AssetManager::GetAsset<Volt::Shader>(item->path);
					if (shader->Reload(true))
					{
						Volt::Renderer::ReloadShader(shader);
						UI::Notify(NotificationType::Success, "Shader Compiled!", std::format("Shader {} compiled succesfully!", item->path.string()));
					}
					else
					{
						UI::Notify(NotificationType::Error, "Shader Compilation Failed", std::format("Shader {} failed to compile!", item->path.string()));
					}*/
				}
			};

			renderFunctions[AssetTypes::MeshSource] = [](AssetItem* item)
			{
				if (ImGui::MenuItem("Import"))
				{
					auto& modal = ModalSystem::GetModal<MeshImportModal>(Sandbox::Get().GetMeshImportModalID());
					modal.SetImportMeshes({ item->path });
					modal.Open();
				}
			};

			renderFunctions[AssetTypes::Mesh] = [](AssetItem* item)
			{
				if (ImGui::MenuItem("Export Mesh"))
				{
					SetMeshExport(item);
					UI::OpenModal(std::format("Mesh Export##assetBrowser{0}", std::to_string(item->handle)));
				}
			};

			renderFunctions[AssetTypes::Animation] = [](AssetItem* item)
			{
				if (ImGui::MenuItem("Reimport"))
				{
					UI::OpenModal(std::format("Reimport Animation##assetBrowser{0}", std::to_string(item->handle)));

				}
			};

			renderFunctions[AssetTypes::Skeleton] = [](AssetItem* item)
			{
			};

			renderFunctions[AssetTypes::Prefab] = [](AssetItem* item)
			{
				if (ImGui::MenuItem("Export Meshes"))
				{
					SetMeshExport(item);
					UI::OpenModal(std::format("Mesh Export##assetBrowser{0}", std::to_string(item->handle)));
				}
			};

			renderFunctions[AssetTypes::TextureSource] = [](AssetItem* item)
			{
				if (ImGui::MenuItem("Import"))
				{
					auto& modal = ModalSystem::GetModal<TextureImportModal>(Sandbox::Get().GetTextureImportModalID());
					modal.SetImportTextures({ item->path });
					modal.Open();
				}
			};

			renderFunctions[AssetTypes::Texture] = [](AssetItem* item)
			{
				if (ImGui::MenuItem("Generate Mips"))
				{
					Ref<Volt::Texture2D> texture = Volt::AssetManager::GetAsset<Volt::Texture2D>(item->handle);
					if (!texture || !texture->IsValid())
					{
						return;
					}

					texture->GetImage()->GenerateMips();
				}
			};
		}

		return renderFunctions;
	}
}
