#include "sbpch.h"
#include "EditorUtilities.h"

#include "Sandbox/Utility/AssetBrowserUtilities.h"
#include "Sandbox/Utility/EditorResources.h"
#include "Sandbox/Utility/Theme.h"

#include <Volt/Asset/Animation/Animation.h>
#include <Volt/Asset/Animation/Skeleton.h>
#include <Volt/Asset/Animation/AnimatedCharacter.h>
#include <Volt/Asset/Animation/AnimationGraphAsset.h>

#include <Volt/Asset/Mesh/Material.h>
#include <Volt/Asset/Mesh/SubMaterial.h>
#include <Volt/Asset/Mesh/MeshCompiler.h>
#include <Volt/Asset/Importers/MeshTypeImporter.h>
#include <Volt/Project/ProjectManager.h>

#include <Volt/Utility/UIUtility.h>

#include <Volt/Rendering/Texture/Texture2D.h>
#include <Volt/Asset/Mesh/Mesh.h>

#include <Volt/Asset/Importers/MeshTypeImporter.h>
#include <Volt/Utility/ImageUtility.h>

#include <stb/stb_image.h>
#include <stb/stb_image_write.h>

#include <DirectXTex/DirectXTex.h>

bool EditorUtils::Property(const std::string& text, Volt::AssetHandle& assetHandle, Volt::AssetType wantedType)
{
	bool changed = false;

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0.f, 0.f });
	UI::BeginPropertyRow();

	ImGui::TextUnformatted(text.c_str());
	ImGui::TableNextColumn();

	std::string assetFileName = "Null";

	const Ref<Volt::Asset> rawAsset = Volt::AssetManager::Get().GetAssetRaw(assetHandle);
	if (rawAsset)
	{
		assetFileName = rawAsset->assetName;

		if (wantedType != Volt::AssetType::None)
		{
			if ((wantedType & rawAsset->GetType()) == Volt::AssetType::None)
			{
				assetHandle = Volt::Asset::Null();
			}
		}
	}

	std::string textId = "##" + std::to_string(UI::GetID());

	changed = UI::DrawItem(ImGui::GetColumnWidth() - 2.f * 25.f, [&]()
	{
		ImGui::InputTextString(textId.c_str(), &assetFileName, ImGuiInputTextFlags_ReadOnly);
		return false;
	});

	if (auto ptr = UI::DragDropTarget("ASSET_BROWSER_ITEM"))
	{
		Volt::AssetHandle newHandle = *(Volt::AssetHandle*)ptr;
		auto droppedAssetType = Volt::AssetManager::GetAssetTypeFromHandle(newHandle);

		if ((droppedAssetType & wantedType) != Volt::AssetType::None)
		{
			assetHandle = newHandle;
			changed = true;
		}
	}

	ImGui::SameLine();

	std::string buttonId = "X##" + std::to_string(UI::GetID());
	if (ImGui::Button(buttonId.c_str(), { 24.5f, 24.5f }))
	{
		assetHandle = Volt::Asset::Null();
		changed = true;
	}

	ImGui::SameLine();

	std::string selectButtonId = "...##" + std::to_string(UI::GetID());
	std::string popupId = "AssetsPopup##" + text + std::to_string(UI::GetID());
	const bool startState = s_assetBrowserPopupsOpen[popupId].state;

	if (ImGui::Button(selectButtonId.c_str(), { 24.5f, 24.5f }))
	{
		ImGui::OpenPopup(popupId.c_str());
		s_assetBrowserPopupsOpen[popupId].state = true;
	}

	if (AssetBrowserPopupInternal(popupId, assetHandle, startState, wantedType))
	{
		changed = true;
	}

	UI::EndPropertyRow();
	ImGui::PopStyleVar();

	return changed;
}

bool EditorUtils::AssetBrowserPopupField(const std::string& id, Volt::AssetHandle& assetHandle, Volt::AssetType wantedType)
{
	const bool startState = s_assetBrowserPopupsOpen[id].state;
	bool changed = false;

	if (startState != ImGui::IsPopupOpen(id.c_str()) && startState == false)
	{
		s_assetBrowserPopupsOpen[id].state = true;
	}

	if (AssetBrowserPopupInternal(id, assetHandle, startState, wantedType))
	{
		changed = true;
	}

	return changed;
}

bool EditorUtils::SearchBar(std::string& outSearchQuery, bool& outHasSearchQuery, bool setAsActive)
{
	UI::ScopedColor childColor{ ImGuiCol_ChildBg, EditorTheme::DarkGreyBackground };
	UI::ScopedStyleFloat rounding(ImGuiStyleVar_ChildRounding, 2.f);

	constexpr float barHeight = 32.f;
	constexpr float searchBarSize = 22.f;
	bool returnVal = false;

	ImGui::BeginChild("##searchBar", { ImGui::GetContentRegionAvail().x, barHeight });
	{
		UI::ShiftCursor(5.f, 4.f);
		ImGui::Image(UI::GetTextureID(EditorResources::GetEditorIcon(EditorIcon::Search)), { searchBarSize, searchBarSize });

		ImGui::SameLine();

		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - ImGui::GetStyle().WindowPadding.x);

		UI::PushID();

		if (setAsActive)
		{
			ImGui::SetKeyboardFocusHere();
		}

		if (UI::InputTextWithHint("", outSearchQuery, "Search..."))
		{
			if (!outSearchQuery.empty())
			{
				outHasSearchQuery = true;
				returnVal = true;
			}
			else
			{
				outHasSearchQuery = false;
			}
		}
		UI::PopID();

		ImGui::PopItemWidth();
	}
	ImGui::EndChild();

	return returnVal;
}

bool EditorUtils::ReimportSourceMesh(Volt::AssetHandle assetHandle, Ref<Volt::Skeleton> targetSkeleton)
{
	const auto assetType = Volt::AssetManager::GetAssetTypeFromHandle(assetHandle);

	if (assetType != Volt::AssetType::Mesh &&
		assetType != Volt::AssetType::Animation &&
		assetType != Volt::AssetType::Skeleton)
	{
		return false;
	}

	// An asset can have multiple dependencies, we must find the source mesh
	std::filesystem::path sourcePath;
	const auto& dependencies = Volt::AssetManager::GetMetadataFromHandle(assetHandle).dependencies;
	for (const auto& d : dependencies)
	{
		const auto& depMeta = Volt::AssetManager::GetMetadataFromHandle(d);
		if (depMeta.filePath.extension().string() == ".fbx")
		{
			sourcePath = depMeta.filePath;
			break;
		}
	}

	if (sourcePath.empty())
	{
		auto meshPath = Volt::AssetManager::GetFilePathFromAssetHandle(assetHandle);
		meshPath.replace_extension(".fbx");

		if (!FileSystem::Exists(Volt::ProjectManager::GetDirectory() / meshPath))
		{
			UI::Notify(NotificationType::Error, "Unable to re import source mesh!", std::format("The source mesh {0} does not exist!", sourcePath.string()));
			return false;
		}

		sourcePath = meshPath;
	}

	if (!FileSystem::Exists(Volt::ProjectManager::GetDirectory() / sourcePath))
	{
		UI::Notify(NotificationType::Error, "Unable to re import source mesh!", std::format("The source mesh {0} does not exist!", sourcePath.string()));
		return false;
	}

	Ref<Volt::Asset> originalAsset = Volt::AssetManager::Get().GetAssetRaw(assetHandle);
	if (!originalAsset || !originalAsset->IsValid())
	{
		UI::Notify(NotificationType::Error, "Unable to re import source mesh!", std::format("The asset {0} is invalid!", originalAsset->assetName));
		return false;
	}

	const auto& origialAssetMeta = Volt::AssetManager::GetMetadataFromHandle(assetHandle);

	if (!FileSystem::IsWriteable(Volt::ProjectManager::GetDirectory() / origialAssetMeta.filePath))
	{
		UI::Notify(NotificationType::Error, "Unable to re import source mesh!", std::format("The asset {0} is not writeable!", originalAsset->assetName));
		return false;
	}

	switch (originalAsset->GetType())
	{
		case Volt::AssetType::Animation:
		{
			Ref<Volt::Animation> newAnim = Volt::MeshTypeImporter::ImportAnimation(Volt::ProjectManager::GetDirectory() / sourcePath, targetSkeleton);
			if (!newAnim)
			{
				UI::Notify(NotificationType::Error, "Unable to re import animation!", std::format("Failed to import animation from {0}!", sourcePath.string()));
				break;
			}

			newAnim->handle = originalAsset->handle;

			Volt::AssetManager::Get().Unload(assetHandle);
			Volt::AssetManager::Get().SaveAsset(newAnim);

			UI::Notify(NotificationType::Success, "Animation re imported!", std::format("Animation {0} was re imported successfully!", sourcePath.string()));

			break;
		}

		case Volt::AssetType::Skeleton:
		{
			Ref<Volt::Skeleton> newSkel = Volt::MeshTypeImporter::ImportSkeleton(Volt::ProjectManager::GetDirectory() / sourcePath);
			if (!newSkel)
			{
				UI::Notify(NotificationType::Error, "Unable to re import skeleton!", std::format("Failed to import skeleton from {0}!", sourcePath.string()));
				break;
			}

			newSkel->handle = originalAsset->handle;

			Volt::AssetManager::Get().Unload(assetHandle);
			Volt::AssetManager::Get().SaveAsset(newSkel);

			UI::Notify(NotificationType::Success, "Skeleton re imported!", std::format("Skeleton {0} was re imported successfully!", sourcePath.string()));

			break;
		}

		case Volt::AssetType::Mesh:
		{
			Ref<Volt::Mesh> originalMesh = std::reinterpret_pointer_cast<Volt::Mesh>(originalAsset);

			Ref<Volt::Mesh> newMesh = Volt::MeshTypeImporter::ImportMesh(Volt::ProjectManager::GetDirectory() / sourcePath);
			if (!newMesh || !newMesh->IsValid())
			{
				UI::Notify(NotificationType::Error, "Unable to re import mesh!", std::format("Failed to import mesh from {0}!", sourcePath.string()));
				break;
			}

			Ref<Volt::Material> originalMaterial = originalMesh->GetMaterial();
			Volt::AssetHandle materialHandle = originalMaterial->handle;
			bool shouldCreateNewMaterial = false;

			if (!originalMaterial || !originalMaterial->IsValid())
			{
				originalMaterial = nullptr;
				shouldCreateNewMaterial = true;
				VT_CORE_WARN("Material for mesh {0} was invalid! Creating a new one!", origialAssetMeta.filePath);
			}
			else
			{
				// We try to check if the material was created with the mesh. This it to make sure that we don't override a shared material
				if (originalMaterial->assetName == originalMesh->assetName)
				{
					if (newMesh->GetMaterial()->GetSubMaterialCount() != originalMaterial->GetSubMaterialCount())
					{
						materialHandle = Volt::Asset::Null();
						shouldCreateNewMaterial = true;

					}
				}
				else
				{
					for (auto& subMesh : newMesh->GetSubMeshesMutable())
					{
						const auto newMat = newMesh->GetMaterial()->GetSubMaterialAt(subMesh.materialIndex);
						const auto& newMatName = newMat->GetName();

						for (const auto& oldMat : originalMaterial->GetSubMaterials())
						{
							const auto& oldMatName = oldMat.second->GetName();

							if (newMatName == oldMatName)
							{
								subMesh.materialIndex = oldMat.first;
								break;
							}
						}
					}
				}
			}

			if (Volt::MeshCompiler::TryCompile(newMesh, origialAssetMeta.filePath, materialHandle))
			{
				Volt::AssetManager::Get().ReloadAsset(originalAsset->handle);

				if (shouldCreateNewMaterial && originalMaterial)
				{
					Ref<Volt::Mesh> reimportedMesh = Volt::AssetManager::GetAsset<Volt::Mesh>(origialAssetMeta.filePath);
					const uint32_t subMaterialCount = glm::min(reimportedMesh->GetMaterial()->GetSubMaterialCount(), originalMaterial->GetSubMaterialCount());

					for (uint32_t i = 0; i < subMaterialCount; i++)
					{
						for (const auto& [binding, tex] : originalMaterial->GetSubMaterials().at(i)->GetTextures())
						{
							// #TODO_Ivar: Reimplement
							//reimportedMesh->GetMaterial()->GetSubMaterials().at(i)->SetTexture(binding, tex);
						}
					}

					Volt::AssetManager::Get().SaveAsset(reimportedMesh->GetMaterial());
					Volt::AssetManager::Get().ReloadAsset(origialAssetMeta.filePath);
				}

				UI::Notify(NotificationType::Success, "Mesh re imported!", std::format("Mesh {0} was re imported successfully!", sourcePath.string()));
			}

			break;
		}
	}

	return true;
}

bool EditorUtils::AssetBrowserPopupInternal(const std::string& popupId, Volt::AssetHandle& assetHandle, bool startState, Volt::AssetType wantedType)
{
	bool changed = false;

	if (auto it = s_assetBrowserPopups.find(popupId); it == s_assetBrowserPopups.end() && s_assetBrowserPopupsOpen[popupId].state != startState)
	{
		s_assetBrowserPopups.emplace(popupId, CreateRef<AssetBrowserPopup>(popupId, wantedType, assetHandle));
	}

	AssetBrowserPopup::State returnState = AssetBrowserPopup::State::Closed;
	if (s_assetBrowserPopups.find(popupId) != s_assetBrowserPopups.end())
	{
		returnState = s_assetBrowserPopups.at(popupId)->Update();
	}

	if (returnState == AssetBrowserPopup::State::Closed || returnState == AssetBrowserPopup::State::Changed)
	{
		s_assetBrowserPopupsOpen[popupId].state = false;
	}

	if (returnState == AssetBrowserPopup::State::Changed)
	{
		changed = true;
	}

	if (startState != s_assetBrowserPopupsOpen[popupId].state && startState == true)
	{
		s_assetBrowserPopups[popupId] = nullptr;
		s_assetBrowserPopups.erase(popupId);
	}

	return changed;
}


ImportState EditorUtils::MeshImportModal(const std::string& aId, MeshImportData& aImportData, const std::filesystem::path& aMeshToImport)
{
	ImportState imported = ImportState::None;

	UI::ScopedStyleFloat rounding{ ImGuiStyleVar_FrameRounding, 2.f };

	if (UI::BeginModal(aId, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_AlwaysAutoResize))
	{
		const std::string srcPath = aMeshToImport.string();

		ImGui::TextUnformatted("Settings");

		UI::PushID();
		if (UI::BeginProperties("Properties"))
		{
			UI::Property("Source", srcPath, true);
			UI::Property("Destination", aImportData.destination);

			UI::EndProperties();
		}
		UI::PopID();

		ImGui::Separator();
		ImGui::TextUnformatted("Mesh");

		UI::PushID();
		if (UI::BeginProperties("Settings"))
		{
			UI::Property("Import Mesh", aImportData.importMesh);
			if (aImportData.importMesh)
			{
				UI::Property("Create Material", aImportData.createMaterial);
			}

			if (!aImportData.createMaterial && aImportData.importMesh)
			{
				EditorUtils::Property("Material", aImportData.externalMaterial, Volt::AssetType::Material);
			}

			UI::EndProperties();
		}

		ImGui::Separator();
		ImGui::TextUnformatted("Animation");

		if (UI::BeginProperties("Settings"))
		{
			UI::Property("Import Skeleton", aImportData.importSkeleton);
			UI::Property("Import Animation", aImportData.importAnimation);

			if (!aImportData.importSkeleton && aImportData.importAnimation)
			{
				EditorUtils::Property("Target Skeleton", aImportData.targetSkeleton, Volt::AssetType::Skeleton);
			}

			UI::EndProperties();
		}
		UI::PopID();

		if (ImGui::Button("Import"))
		{
			const Volt::AssetHandle material = aImportData.createMaterial ? Volt::Asset::Null() : aImportData.externalMaterial;
			bool succeded = false;

			if (aImportData.importMesh)
			{
				Ref<Volt::Mesh> importMesh = Volt::AssetManager::GetAsset<Volt::Mesh>(aMeshToImport);
				if (importMesh && importMesh->IsValid())
				{
					if (!aImportData.createMaterial)
					{
						const auto importedMaterial = importMesh->GetMaterial();
						const auto materialToUse = Volt::AssetManager::GetAsset<Volt::Material>(aImportData.externalMaterial);
						if (materialToUse && materialToUse->IsValid())
						{
							for (auto& subMesh : importMesh->GetSubMeshesMutable())
							{
								auto currentSubMeshMat = importedMaterial->GetSubMaterialAt(subMesh.materialIndex);

								for (const auto& [index, subMat] : materialToUse->GetSubMaterials())
								{
									if (currentSubMeshMat->GetName() == subMat->GetName())
									{
										subMesh.materialIndex = index;
										break;
									}
								}
							}
						}
					}

					succeded = Volt::MeshCompiler::TryCompile(importMesh, aImportData.destination, material);
					if (!succeded)
					{
						UI::Notify(NotificationType::Error, "Failed to compile mesh!", std::format("Failed to compile mesh to location {}!", aImportData.destination.string()));
					}

					auto handle = Volt::AssetManager::Get().AddAssetToRegistry(aImportData.destination);
					Volt::AssetManager::Get().AddDependency(handle, aMeshToImport);
				}
				else
				{
					UI::Notify(NotificationType::Error, "Failed to compile mesh!", std::format("Failed to compile mesh to location {}!", aImportData.destination.string()));
				}

				if (importMesh)
				{
					Volt::AssetManager::Get().Unload(importMesh->handle);
				}
			}

			if (aImportData.importSkeleton)
			{
				Ref<Volt::Skeleton> skeleton = Volt::MeshTypeImporter::ImportSkeleton(Volt::ProjectManager::GetDirectory() / aMeshToImport);
				if (!skeleton)
				{
					UI::Notify(NotificationType::Error, "Failed to import skeleton!", std::format("Failed to import skeleton from {}!", aMeshToImport.string()));
				}
				else
				{
					const std::filesystem::path filePath = aImportData.destination.parent_path() / (aImportData.destination.stem().string() + ".vtsk");

					Volt::AssetManager::Get().SaveAssetAs(skeleton, filePath);
					Volt::AssetManager::Get().AddDependency(skeleton->handle, aMeshToImport);
					succeded = true;
				}
			}

			if (aImportData.importAnimation)
			{
				Ref<Volt::Skeleton> targetSkeleton = Volt::AssetManager::GetAsset<Volt::Skeleton>(aImportData.targetSkeleton);
				if (!targetSkeleton || !targetSkeleton->IsValid())
				{
					UI::Notify(NotificationType::Error, "Failed to import animation!", std::format("Skeleton with handle {0} is invalid!", static_cast<uint64_t>(aImportData.targetSkeleton)));
				}
				else
				{
					Ref<Volt::Animation> animation = Volt::MeshTypeImporter::ImportAnimation(Volt::ProjectManager::GetDirectory() / aMeshToImport, targetSkeleton);
					if (!animation)
					{
						UI::Notify(NotificationType::Error, "Failed to import animation!", std::format("Failed to import animaition from {}!", aMeshToImport.string()));
					}
					else
					{
						const std::filesystem::path filePath = aImportData.destination.parent_path() / (aImportData.destination.stem().string() + ".vtanim");
						Volt::AssetManager::Get().SaveAssetAs(animation, filePath);
						Volt::AssetManager::Get().AddDependency(animation->handle, aMeshToImport);
						succeded = true;
					}
				}
			}

			if (succeded)
			{
				UI::Notify(NotificationType::Success, "Mesh compilation succeded!", std::format("Successfully compiled mesh to {}!", aImportData.destination.string()));
				imported = ImportState::Imported;
			}

			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel"))
		{
			imported = ImportState::Discard;
			ImGui::CloseCurrentPopup();
		}

		UI::EndModal();
	}
	return imported;
}

ImportState EditorUtils::MeshBatchImportModal(const std::string& aId, MeshImportData& aImportData, const std::vector<std::filesystem::path>& meshesToImport)
{
	ImportState imported = ImportState::None;

	UI::ScopedStyleFloat rounding{ ImGuiStyleVar_FrameRounding, 2.f };

	if (UI::BeginModal(aId, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::TextUnformatted("Settings");

		UI::PushID();
		if (UI::BeginProperties("Properties"))
		{
			UI::Property("Destination Directory", aImportData.destination);

			UI::EndProperties();
		}
		UI::PopID();

		ImGui::Separator();
		ImGui::TextUnformatted("Mesh");

		UI::PushID();
		if (UI::BeginProperties("Settings"))
		{
			UI::Property("Create Materials", aImportData.createMaterial);

			if (!aImportData.createMaterial)
			{
				EditorUtils::Property("Material", aImportData.externalMaterial, Volt::AssetType::Material);
			}

			UI::EndProperties();
		}
		UI::PopID();

		if (ImGui::Button("Import"))
		{
			for (const auto& src : meshesToImport)
			{
				const Volt::AssetHandle material = aImportData.createMaterial ? Volt::Asset::Null() : aImportData.externalMaterial;
				const std::filesystem::path destinationPath = aImportData.destination / (src.stem().string() + ".vtmesh");

				bool succeded = false;

				Ref<Volt::Mesh> importMesh = Volt::AssetManager::GetAsset<Volt::Mesh>(src);
				if (importMesh && importMesh->IsValid())
				{
					if (!aImportData.createMaterial)
					{
						const auto importedMaterial = importMesh->GetMaterial();
						const auto materialToUse = Volt::AssetManager::GetAsset<Volt::Material>(aImportData.externalMaterial);
						if (materialToUse && materialToUse->IsValid())
						{
							for (auto& subMesh : importMesh->GetSubMeshesMutable())
							{
								auto currentSubMeshMat = importedMaterial->GetSubMaterialAt(subMesh.materialIndex);

								for (const auto& [index, subMat] : materialToUse->GetSubMaterials())
								{
									if (currentSubMeshMat->GetName() == subMat->GetName())
									{
										subMesh.materialIndex = index;
										break;
									}
								}
							}
						}
					}

					succeded = Volt::MeshCompiler::TryCompile(importMesh, destinationPath, material);
					if (!succeded)
					{
						UI::Notify(NotificationType::Error, "Failed to compile mesh!", std::format("Failed to compile mesh to location {}!", destinationPath.string()));
					}

					auto handle = Volt::AssetManager::Get().AddAssetToRegistry(destinationPath);
					Volt::AssetManager::Get().AddDependency(handle, src);
				}
				else
				{
					UI::Notify(NotificationType::Error, "Failed to compile mesh!", std::format("Failed to compile mesh to location {}!", destinationPath.string()));
				}

				if (importMesh)
				{
					Volt::AssetManager::Get().Unload(importMesh->handle);
				}

				if (succeded)
				{
					UI::Notify(NotificationType::Success, "Mesh compilation succeded!", std::format("Successfully compiled mesh to {}!", destinationPath.string()));
					imported = ImportState::Imported;
				}
			}

			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel"))
		{
			imported = ImportState::Discard;
			ImGui::CloseCurrentPopup();
		}

		UI::EndModal();
	}
	return imported;
}

void EditorUtils::MeshExportModal(const std::string& aId, std::filesystem::path aDirectoryPath, MeshImportData& aExportData, std::vector<Ref<Volt::Mesh>> aMeshesToExport)
{
	if (aMeshesToExport.empty()) { return; }

	static std::string path;

	UI::ScopedStyleFloat rounding{ ImGuiStyleVar_FrameRounding, 2.f };

	if (UI::BeginModal(aId, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_AlwaysAutoResize))
	{
		UI::InputText("Path", path);

		if (ImGui::Button("Export"))
		{
			Volt::MeshTypeImporter::ExportMesh(aMeshesToExport, aDirectoryPath / path);
			path = "";
			AssetBrowser::AssetBrowserUtilities::ResetMeshExport();
			ImGui::CloseCurrentPopup();
		}

		if (ImGui::Button("Cancel"))
		{
			path = "";
			AssetBrowser::AssetBrowserUtilities::ResetMeshExport();
			ImGui::CloseCurrentPopup();
		}

		UI::EndModal();
	}
}

void EditorUtils::ImportTexture(const std::filesystem::path& sourcePath)
{
	//DirectX::TexMetadata metadata{};
	//DirectX::ScratchImage scratchImage{};

	//if (sourcePath.extension().string() == ".tga")
	//{
	//	DirectX::LoadFromTGAFile(sourcePath.c_str(), &metadata, scratchImage);
	//	metadata.format = DXGI_FORMAT_R8G8B8A8_UNORM;
	//}
	//else if (sourcePath.extension().string() == ".hdr")
	//{
	//	DirectX::LoadFromHDRFile(sourcePath.c_str(), &metadata, scratchImage);
	//	metadata.format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	//}
	//else
	//{
	//	DirectX::LoadFromWICFile(sourcePath.c_str(), DirectX::WIC_FLAGS_NONE, &metadata, scratchImage);
	//	metadata.format = DXGI_FORMAT_R8G8B8A8_UNORM;
	//}

	//const uint32_t mipCount = Volt::Utility::CalculateMipCount((uint32_t)metadata.width, (uint32_t)metadata.height);

	//DirectX::Image image{};
	//image.width = metadata.width;
	//image.height = metadata.height;
	//image.format = metadata.format;
	//image.pixels = scratchImage.GetImage(0, 0, 0)->pixels;

	//DirectX::ScratchImage mippedImage{};
	//DirectX::GenerateMipMaps(image, DirectX::TEX_FILTER_CUBIC, (size_t)mipCount, mippedImage);

	//std::vector<DirectX::Image> dxImages{};

	//uint32_t mipWidth = metadata.width;
	//uint32_t mipHeight = metadata.height;

	//uint32_t offset = 0;
	//for (uint32_t mip = 0; mip < mipCount; mip++)
	//{
	//	const uint32_t mipSize = mipWidth * mipHeight * Volt::Utility::PerPixelSizeFromFormat(Volt::Utility::DXToVoltFormat(metadata.format));

	//	DirectX::Image dxImage{};
	//	dxImage.width = mipWidth;
	//	dxImage.height = mipHeight;
	//	dxImage.format = metadata.format;
	//	dxImage.rowPitch = mipWidth * Volt::Utility::PerPixelSizeFromFormat(Volt::Utility::DXToVoltFormat(metadata.format));
	//	dxImage.slicePitch = mipSize;
	//	dxImage.pixels = mippedImage.GetImage(mip, 0, 0)->pixels;

	//	offset += mipSize;

	//	mipWidth = std::max(1u, mipWidth / 2);
	//	mipHeight = std::max(1u, mipHeight / 2);

	//	dxImages.emplace_back(dxImage);
	//}

	//metadata.mipLevels = (uint32_t)mipCount;

	//DXGI_FORMAT compressFormat = DXGI_FORMAT_BC7_UNORM;

	//if (sourcePath.filename().string().contains("_c."))
	//{
	//	compressFormat = DXGI_FORMAT_BC7_UNORM_SRGB;
	//}

	//const auto destPath = sourcePath.parent_path() / (sourcePath.stem().string() + ".dds");

	//DirectX::ScratchImage compressedImage{};
	//DirectX::Compress(Volt::GraphicsContext::GetDevice().Get(), dxImages.data(), dxImages.size(), metadata, compressFormat, DirectX::TEX_COMPRESS_DEFAULT, DirectX::TEX_THRESHOLD_DEFAULT, compressedImage);
	//DirectX::SaveToDDSFile(compressedImage.GetImages(), compressedImage.GetImageCount(), compressedImage.GetMetadata(), DirectX::DDS_FLAGS_NONE, destPath.c_str());
}

bool EditorUtils::NewCharacterModal(const std::string& aId, Ref<Volt::AnimatedCharacter>& outCharacter, NewCharacterData& aCharacterData)
{
	bool created = false;

	UI::ScopedStyleFloat rounding{ ImGuiStyleVar_FrameRounding, 2.f };
	if (UI::BeginModal(aId, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
	{
		UI::ShiftCursor(300.f, 0.f);
		UI::ShiftCursor(-300.f, 0.f);

		if (UI::BeginProperties("NewCharacter"))
		{
			UI::Property("Name", aCharacterData.name);
			EditorUtils::Property("Skeleton", aCharacterData.skeletonHandle, Volt::AssetType::Skeleton);
			EditorUtils::Property("Skin", aCharacterData.skinHandle, Volt::AssetType::Mesh);
			UI::PropertyDirectory("Destination", aCharacterData.destination);

			UI::EndProperties();
		}

		if (ImGui::Button("Cancel"))
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		if (ImGui::Button("Create"))
		{
			created = true;
			outCharacter = Volt::AssetManager::CreateAsset<Volt::AnimatedCharacter>(aCharacterData.destination, aCharacterData.name + ".vtchr");

			if (aCharacterData.skeletonHandle != Volt::Asset::Null())
			{
				outCharacter->SetSkeleton(Volt::AssetManager::GetAsset<Volt::Skeleton>(aCharacterData.skeletonHandle));
			}

			if (aCharacterData.skinHandle != Volt::Asset::Null())
			{
				outCharacter->SetSkin(Volt::AssetManager::GetAsset<Volt::Mesh>(aCharacterData.skinHandle));
			}

			Volt::AssetManager::Get().SaveAsset(outCharacter);
			ImGui::CloseCurrentPopup();
		}

		UI::EndModal();
	}

	return created;
}

bool EditorUtils::NewAnimationGraphModal(const std::string& aId, Ref<Volt::AnimationGraphAsset>* outGraph, NewAnimationGraphData& graphData)
{
	bool created = false;

	UI::ScopedStyleFloat rounding{ ImGuiStyleVar_FrameRounding, 2.f };
	if (UI::BeginModal(aId, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
	{
		UI::ShiftCursor(300.f, 0.f);
		UI::ShiftCursor(-300.f, 0.f);

		if (UI::BeginProperties("NewGraph"))
		{
			UI::Property("Name", graphData.name);
			EditorUtils::Property("Character", graphData.characterHandle, Volt::AssetType::AnimatedCharacter);
			UI::PropertyDirectory("Destination", graphData.destination);

			UI::EndProperties();
		}

		if (ImGui::Button("Cancel"))
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		if (ImGui::Button("Create"))
		{
			if (graphData.characterHandle == Volt::Asset::Null())
			{
				UI::Notify(NotificationType::Error, "Unable to create animation graph!", "Character must not be null!");

				UI::EndModal();
				return false;
			}

			created = true;
			auto newGraph = Volt::AssetManager::CreateAsset<Volt::AnimationGraphAsset>(graphData.destination, graphData.name + ".vtanimgraph", graphData.characterHandle);

			if (outGraph)
			{
				*outGraph = newGraph;
			}

			Volt::AssetManager::Get().SaveAsset(newGraph);
			ImGui::CloseCurrentPopup();
		}

		UI::EndModal();
	}

	return created;
}

SaveReturnState EditorUtils::SaveFilePopup(const std::string& aId)
{
	SaveReturnState returnState = SaveReturnState::None;
	UI::ScopedStyleFloat rounding{ ImGuiStyleVar_FrameRounding, 2.f };

	if (UI::BeginModal(aId, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
	{
		if (ImGui::Button("Cancel"))
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		if (ImGui::Button("Discard"))
		{
			returnState = SaveReturnState::Discard;
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		if (ImGui::Button("Save"))
		{
			returnState = SaveReturnState::Save;
			ImGui::CloseCurrentPopup();
		}

		UI::EndModal();
	}

	return returnState;
}

Ref<Volt::Texture2D> EditorUtils::GenerateThumbnail(const std::filesystem::path& path)
{
	Ref<Volt::Texture2D> srcTexture = Volt::AssetManager::GetAsset<Volt::Texture2D>(path);
	if (!srcTexture || !srcTexture->IsValid())
	{
		return nullptr;
	}

	//Volt::RenderPass renderPass;
	//Volt::FramebufferSpecification spec{};
	//spec.width = 128;
	//spec.height = 128;
	//spec.attachments =
	//{
	//	{ Volt::ImageFormat::RGBA }
	//};

	//renderPass.framebuffer = Volt::Framebuffer::Create(spec);

	//Volt::Renderer::BeginFullscreenPass(renderPass, nullptr);

	//// #TODO_Ivar: Reimplement

	////Volt::Renderer::BindTexturesToStage(Volt::ShaderStage::Pixel, { srcTexture->GetImage() }, 0);
	//Volt::Renderer::DrawFullscreenTriangleWithShader(Volt::ShaderRegistry::Get("CopyTextureToTarget"));
	//Volt::Renderer::EndFullscreenPass();

	//const std::filesystem::path thumbnailPath = GetThumbnailPathFromPath(path);
	//Ref<Volt::Texture2D> thumbnailAsset = Volt::AssetManager::CreateAsset<Volt::Texture2D>(thumbnailPath.parent_path(), thumbnailPath.filename().string());

	//Volt::Renderer::SubmitPostExcecution([=]()
	//	{
	//		const Ref<Volt::Image2D> image = renderPass.framebuffer->GetColorAttachment(0);
	//		const auto& imageSpec = image->GetSpecification();

	//		Volt::Buffer buffer = renderPass.framebuffer->GetColorAttachment(0)->GetDataBuffer();

	//		constexpr int32_t channels = 4;
	//		stbi_write_png((Volt::ProjectManager::GetDirectory() / thumbnailPath).string().c_str(), imageSpec.width, imageSpec.height, channels, buffer.As<void>(), imageSpec.width * Volt::Utility::PerPixelSizeFromFormat(imageSpec.format));

	//		//thumbnailAsset->SetImage(image);
	//		buffer.Release();
	//	});


	return nullptr;
}

bool EditorUtils::HasThumbnail(const std::filesystem::path& path)
{
	return FileSystem::Exists(Volt::ProjectManager::GetDirectory() / GetThumbnailPathFromPath(path));
}

std::filesystem::path EditorUtils::GetThumbnailPathFromPath(const std::filesystem::path& path)
{
	return path.string() + ".vtthumb.png";
}
