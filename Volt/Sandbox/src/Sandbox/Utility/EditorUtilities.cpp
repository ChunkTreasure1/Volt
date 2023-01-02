#include "sbpch.h"
#include "EditorUtilities.h"

#include "Sandbox/Utility/EditorResources.h"

#include <Volt/Asset/Animation/Animation.h>
#include <Volt/Asset/Animation/Skeleton.h>
#include <Volt/Asset/Animation/AnimatedCharacter.h>

#include <Volt/Asset/Mesh/MeshCompiler.h>
#include <Volt/Asset/Importers/MeshTypeImporter.h>
#include <Volt/Project/ProjectManager.h>

#include <Volt/Utility/UIUtility.h>

#include <Volt/Rendering/Texture/Texture2D.h>
#include <Volt/Asset/Mesh/Mesh.h>
#include <Volt/Rendering/Renderer.h>
#include <Volt/Rendering/Framebuffer.h>
#include <Volt/Rendering/Shader/ShaderRegistry.h>

#include <stb/stb_image.h>
#include <stb/stb_image_write.h>

bool EditorUtils::Property(const std::string& text, Volt::AssetHandle& assetHandle, Volt::AssetType wantedType /* = Volt::AssetType::None */, std::function<void(Volt::AssetHandle& value)> callback /* = nullptr */)
{
	bool changed = false;

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0.f, 0.f });

	ImGui::TableNextColumn();
	ImGui::TextUnformatted(text.c_str());

	ImGui::TableNextColumn();

	ImGui::PushItemWidth(ImGui::GetColumnWidth() - 2.f * 25.f);

	std::string assetFileName = "Null";

	const Ref<Volt::Asset> rawAsset = Volt::AssetManager::Get().GetAssetRaw(assetHandle);
	if (rawAsset)
	{
		assetFileName = rawAsset->path.filename().string();

		if (wantedType != Volt::AssetType::None)
		{
			if (wantedType != rawAsset->GetType())
			{
				assetHandle = Volt::Asset::Null();
			}
		}
	}


	std::string textId = "##" + std::to_string(UI::GetId());
	ImGui::InputTextString(textId.c_str(), &assetFileName, ImGuiInputTextFlags_ReadOnly);
	ImGui::PopItemWidth();

	if (auto ptr = UI::DragDropTarget("ASSET_BROWSER_ITEM"))
	{
		Volt::AssetHandle newHandle = *(Volt::AssetHandle*)ptr;
		assetHandle = newHandle;
		changed = true;
		if (callback)
		{
			callback(assetHandle);
		}
	}

	ImGui::SameLine();

	std::string buttonId = "X##" + std::to_string(UI::GetId());
	if (ImGui::Button(buttonId.c_str(), { 24.5f, 24.5f }))
	{
		assetHandle = Volt::Asset::Null();
		changed = true;
		if (callback)
		{
			callback(assetHandle);
		}
	}

	ImGui::SameLine();


	std::string selectButtonId = "...##" + std::to_string(UI::GetId());
	std::string popupId = "AssetsPopup##" + text + std::to_string(UI::GetId());
	const bool startState = s_assetBrowserPopupsOpen[popupId].state;

	if (ImGui::Button(selectButtonId.c_str(), { 24.5f, 24.5f }))
	{
		ImGui::OpenPopup(popupId.c_str());
		s_assetBrowserPopupsOpen[popupId].state = true;
	}

	if (AssetBrowserPopupInternal(popupId, assetHandle, startState, wantedType, callback))
	{
		changed = true;
	}

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

bool EditorUtils::SearchBar(std::string& outSearchQuery, bool& outHasSearchQuery)
{
	UI::ScopedColor childColor{ ImGuiCol_ChildBg, { 0.2f, 0.2f, 0.2f, 1.f } };
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

		UI::PushId();
		if (UI::InputText("", outSearchQuery))
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
		UI::PopId();

		ImGui::PopItemWidth();
	}
	ImGui::EndChild();

	return returnVal;
}

bool EditorUtils::AssetBrowserPopupInternal(const std::string& popupId, Volt::AssetHandle& assetHandle, bool startState, Volt::AssetType wantedType, std::function<void(Volt::AssetHandle& value)> callback)
{
	bool changed = false;

	if (auto it = s_assetBrowserPopups.find(popupId); it == s_assetBrowserPopups.end() && s_assetBrowserPopupsOpen[popupId].state != startState)
	{
		s_assetBrowserPopups.emplace(popupId, CreateRef<AssetBrowserPopup>(popupId, wantedType, assetHandle, callback));
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

		UI::PushId();
		if (UI::BeginProperties("Properties"))
		{
			UI::Property("Source", srcPath, true);
			UI::Property("Destination", aImportData.destination);

			UI::EndProperties();
		}
		UI::PopId();

		ImGui::Separator();
		ImGui::TextUnformatted("Mesh");

		UI::PushId();
		if (UI::BeginProperties("Settings"))
		{
			UI::Property("Import Mesh", aImportData.importMesh);
			if (aImportData.importMesh)
			{
				UI::Property("Create Materials", aImportData.createMaterials);
			}

			if (!aImportData.createMaterials && aImportData.importMesh)
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

			UI::EndProperties();
		}
		UI::PopId();

		if (ImGui::Button("Import"))
		{
			const Volt::AssetHandle material = aImportData.createMaterials ? Volt::Asset::Null() : aImportData.externalMaterial;
			bool succeded = true;

			if (aImportData.importMesh)
			{
				Ref<Volt::Mesh> importMesh = Volt::AssetManager::GetAsset<Volt::Mesh>(aMeshToImport);
				if (importMesh && importMesh->IsValid())
				{
					succeded = succeded && Volt::MeshCompiler::TryCompile(importMesh, aImportData.destination, material);
					if (!succeded)
					{
						UI::Notify(NotificationType::Error, "Failed to compile mesh!", std::format("Failed to compile mesh to location {}!", aImportData.destination.string()));
					}
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
				Ref<Volt::Skeleton> skeleton = Volt::MeshTypeImporter::ImportSkeleton(aMeshToImport);
				if (!skeleton)
				{
					UI::Notify(NotificationType::Error, "Failed to import skeleton!", std::format("Failed to import skeleton from {}!", aMeshToImport.string()));
				}
				else
				{
					skeleton->path = aImportData.destination.parent_path() / (aImportData.destination.stem().string() + ".vtsk");
					Volt::AssetManager::Get().SaveAsset(skeleton);
				}
			}

			if (aImportData.importAnimation)
			{
				Ref<Volt::Animation> animation = Volt::MeshTypeImporter::ImportAnimation(aMeshToImport);
				if (!animation)
				{
					UI::Notify(NotificationType::Error, "Failed to import animation!", std::format("Failed to import animaition from {}!", aMeshToImport.string()));
				}
				else
				{
					animation->path = aImportData.destination.parent_path() / (aImportData.destination.stem().string() + ".vtanim");
					Volt::AssetManager::Get().SaveAsset(animation);
				}
			}

			if (succeded)
			{
				UI::Notify(NotificationType::Success, "Mesh compilation succeded!", std::format("Successfully compiled mesh to {}!", aImportData.destination.string()));
				imported = ImportState::Imported;
			}
			else
			{
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
			outCharacter = Volt::AssetManager::CreateAsset<Volt::AnimatedCharacter>(Volt::ProjectManager::GetPathRelativeToProject(aCharacterData.destination), aCharacterData.name + ".vtchr");

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

	Volt::RenderPass renderPass;
	Volt::FramebufferSpecification spec{};
	spec.width = 512;
	spec.height = 512;
	spec.attachments =
	{
		{ Volt::ImageFormat::RGBA }
	};

	renderPass.framebuffer = Volt::Framebuffer::Create(spec);

	Volt::Renderer::BeginFullscreenPass(renderPass, nullptr);
	auto context = Volt::GraphicsContext::GetContext();
	context->PSSetShaderResources(0, 1, srcTexture->GetImage()->GetSRV().GetAddressOf());

	Volt::Renderer::DrawFullscreenTriangleWithShader(Volt::ShaderRegistry::Get("CopyTextureToTarget"));
	Volt::Renderer::EndFullscreenPass();

	const Ref<Volt::Image2D> image = renderPass.framebuffer->GetColorAttachment(0);
	const auto& imageSpec = image->GetSpecification();

	Volt::Buffer buffer = renderPass.framebuffer->GetColorAttachment(0)->GetDataBuffer();
	const std::filesystem::path thumbnailPath = GetThumbnailPathFromPath(path);

	constexpr int32_t channels = 4;
	stbi_write_png((Volt::ProjectManager::GetPath() / thumbnailPath).string().c_str(), imageSpec.width, imageSpec.height, channels, buffer.As<void>(), imageSpec.width * Volt::Utility::PerPixelSizeFromFormat(imageSpec.format));
	Ref<Volt::Texture2D> thumbnailAsset = Volt::AssetManager::CreateAsset<Volt::Texture2D>(thumbnailPath.parent_path(), thumbnailPath.filename().string(), image);

	buffer.Release();

	return thumbnailAsset;
}

bool EditorUtils::HasThumbnail(const std::filesystem::path& path)
{
	return FileSystem::Exists(Volt::ProjectManager::GetPath() / GetThumbnailPathFromPath(path));
}

std::filesystem::path EditorUtils::GetThumbnailPathFromPath(const std::filesystem::path& path)
{
	return path.string() + ".vtthumb.png";
}
