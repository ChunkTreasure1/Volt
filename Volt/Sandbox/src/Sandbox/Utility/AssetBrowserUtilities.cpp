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
				if (ImGui::MenuItem("Recompile Shaders"))
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

					UI::OpenModal("Import Mesh##assetBrowser");
				}
			};

			renderFunctions[Volt::AssetType::Mesh] = [](AssetItem* item)
			{
				if (ImGui::MenuItem("Reimport"))
				{
					const std::filesystem::path fbxPath = item->path.parent_path() / (item->path.stem().string() + ".fbx");
					if (FileSystem::Exists(fbxPath))
					{
						Ref<Volt::Mesh> currentMesh = Volt::AssetManager::GetAsset<Volt::Mesh>(item->handle);
						if (currentMesh)
						{
							if (!FileSystem::IsWriteable(currentMesh->path))
							{
								UI::Notify(NotificationType::Error, "Unable to Compile Mesh!", std::format("Mesh {0} is not writeable!", item->path.string()), 5000);
								return;
							}

							Ref<Volt::Material> material;
							if (currentMesh->GetMaterial() && currentMesh->GetMaterial()->IsValid())
							{
								material = currentMesh->GetMaterial();
							}
							else
							{
								VT_CORE_WARN("Material for mesh {0} was invalid! Creating a new one!", currentMesh->path);
							}

							Volt::AssetHandle materialHandle = material ? material->handle : Volt::Asset::Null();
							bool recreatedMaterial = false;

							auto newFbxMesh = Volt::AssetManager::GetAsset<Volt::Mesh>(fbxPath);
							if (newFbxMesh && newFbxMesh->IsValid())
							{
								if (newFbxMesh->GetMaterial()->GetSubMaterials().size() != material->GetSubMaterials().size())
								{
									materialHandle = Volt::Asset::Null();
									recreatedMaterial = true;
								}

								if (Volt::MeshCompiler::TryCompile(newFbxMesh, item->path, materialHandle))
								{
									UI::Notify(NotificationType::Success, "Mesh Compiled!", std::format("Mesh {0} compiled successfully!", fbxPath.string()));
									Volt::AssetManager::Get().ReloadAsset(item->path);

									if (recreatedMaterial && material)
									{
										auto newMesh = Volt::AssetManager::GetAsset<Volt::Mesh>(item->path);
										const size_t subMaterialCount = gem::min(newMesh->GetMaterial()->GetSubMaterials().size(), material->GetSubMaterials().size());

										// Set textures from old material
										for (size_t i = 0; i < subMaterialCount; i++)
										{
											for (const auto& [binding, tex] : material->GetSubMaterials().at(i)->GetTextures())
											{
												newMesh->GetMaterial()->GetSubMaterials().at(i)->SetTexture(binding, tex);
											}
										}

										Volt::AssetManager::Get().SaveAsset(newMesh->GetMaterial());
										Volt::AssetManager::Get().ReloadAsset(newMesh->GetMaterial()->path);
									}
								}
								else
								{
									UI::Notify(NotificationType::Error, "Mesh Failed to Compile!", std::format("Mesh {0} failed to compile!", fbxPath.string()));
								}
							}
							else
							{
								UI::Notify(NotificationType::Error, "Mesh Failed to Compile!", std::format("Mesh {0} failed to compile!", fbxPath.string()));
							}

							if (newFbxMesh)
							{
								Volt::AssetManager::Get().Unload(newFbxMesh->handle);
							}
						}
						else
						{
							UI::Notify(NotificationType::Error, "Unable to Compile Mesh!", std::format("Mesh {0} is invalid or cannot be found!", item->path.string()));
						}
					}
				}
			};
			
			renderFunctions[Volt::AssetType::Animation] = [](AssetItem* item)
			{
				if (ImGui::MenuItem("Reimport"))
				{
					const std::filesystem::path fbxPath = item->path.parent_path() / (item->path.stem().string() + ".fbx");
					if (FileSystem::Exists(fbxPath))
					{
						Ref<Volt::Animation> originalAnimation = Volt::AssetManager::GetAsset<Volt::Animation>(item->handle);
						if (originalAnimation)
						{
							if (!FileSystem::IsWriteable(originalAnimation->path))
							{
								UI::Notify(NotificationType::Error, "Unable to Reimport Animation!", std::format("Animation {0} is not writeable!", item->path.string()), 5000);
								return;
							}

							Ref<Volt::Animation> newAnimation = Volt::MeshTypeImporter::ImportAnimation(fbxPath);
							if (newAnimation && newAnimation->IsValid())
							{
								newAnimation->handle = originalAnimation->handle;
								newAnimation->path = originalAnimation->path;

								Volt::AssetManager::Get().Unload(item->handle);
								Volt::AssetManager::Get().SaveAsset(newAnimation);

								UI::Notify(NotificationType::Success, "Animation Re imported!", std::format("Animation {0} was re imported successfully!", fbxPath.string()));
							}
							else
							{
								UI::Notify(NotificationType::Error, "Failed to Reimport Animation!", std::format("Animation {0} failed to import!", fbxPath.string()));
							}
						}
						else
						{
							UI::Notify(NotificationType::Error, "Failed to Reimport Animation!", std::format("Animation {0} failed was not found or is invalid!", item->path.string()));
						}
					}
					else
					{
						UI::Notify(NotificationType::Error, "Failed to Reimport Animation!", std::format("Animation {0} failed was not found!", fbxPath.string()));
					}
				}
			};

			renderFunctions[Volt::AssetType::Skeleton] = [](AssetItem* item)
			{
				if (ImGui::MenuItem("Reimport"))
				{
					const std::filesystem::path fbxPath = item->path.parent_path() / (item->path.stem().string() + ".fbx");
					if (FileSystem::Exists(fbxPath))
					{
						Ref<Volt::Skeleton> originalSkeleton = Volt::AssetManager::GetAsset<Volt::Skeleton>(item->handle);
						if (originalSkeleton)
						{
							if (!FileSystem::IsWriteable(originalSkeleton->path))
							{
								UI::Notify(NotificationType::Error, "Unable to Reimport Skeleton!", std::format("Skeleton {0} is not writeable!", item->path.string()), 5000);
								return;
							}

							Ref<Volt::Skeleton> newSkeleton = Volt::MeshTypeImporter::ImportSkeleton(fbxPath);
							if (newSkeleton && newSkeleton->IsValid())
							{
								newSkeleton->handle = originalSkeleton->handle;
								newSkeleton->path = originalSkeleton->path;

								Volt::AssetManager::Get().Unload(item->handle);
								Volt::AssetManager::Get().SaveAsset(newSkeleton);

								UI::Notify(NotificationType::Success, "Skeleton Re imported!", std::format("Skeleton {0} was reimported successfully!", fbxPath.string()));
							}
							else
							{
								UI::Notify(NotificationType::Error, "Failed to Reimport Skeleton!", std::format("Skeleton {0} failed to import!", fbxPath.string()));
							}
						}
						else
						{
							UI::Notify(NotificationType::Error, "Failed to Reimport Skeleton!", std::format("Skeleton {0} failed was not found or is invalid!", item->path.string()));
						}
					}
					else
					{
						UI::Notify(NotificationType::Error, "Failed to Reimport Skeleton!", std::format("Skeleton {0} failed was not found or is invalid!", fbxPath.string()));
					}
				}
			};
		}

		return renderFunctions;
	}
}