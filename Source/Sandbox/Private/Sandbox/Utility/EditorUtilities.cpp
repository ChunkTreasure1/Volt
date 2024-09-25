#include "sbpch.h"
#include "Utility/EditorUtilities.h"

#include "Sandbox/Utility/AssetBrowserUtilities.h"
#include "Sandbox/Utility/EditorResources.h"
#include "Sandbox/Utility/Theme.h"

#include <Volt/Asset/Animation/Animation.h>
#include <Volt/Asset/Animation/Skeleton.h>
#include <Volt/Asset/Animation/AnimatedCharacter.h>

#include <Volt/Asset/Rendering/Material.h>
#include <Volt/Asset/Mesh/MeshSource.h>
#include <Volt/Project/ProjectManager.h>

#include <Volt/Utility/UIUtility.h>

#include <Volt/Rendering/Texture/Texture2D.h>
#include <Volt/Asset/Mesh/Mesh.h>

#include <stb/stb_image.h>
#include <stb/stb_image_write.h>

bool EditorUtils::Property(const std::string& text, Volt::AssetHandle& assetHandle, AssetType wantedType)
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

		if (wantedType != AssetTypes::None && wantedType != rawAsset->GetType())
		{
			assetHandle = Volt::Asset::Null();
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

		if (droppedAssetType == wantedType)
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

bool EditorUtils::AssetBrowserPopupField(const std::string& id, Volt::AssetHandle& assetHandle, AssetType wantedType)
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

bool EditorUtils::AssetBrowserPopupInternal(const std::string& popupId, Volt::AssetHandle& assetHandle, bool startState, AssetType wantedType)
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
			EditorUtils::Property("Skeleton", aCharacterData.skeletonHandle, AssetTypes::Skeleton);
			EditorUtils::Property("Skin", aCharacterData.skinHandle, AssetTypes::Mesh);
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
			outCharacter = Volt::AssetManager::CreateAsset<Volt::AnimatedCharacter>(aCharacterData.destination, aCharacterData.name);

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
	return FileSystem::Exists(Volt::ProjectManager::GetRootDirectory() / GetThumbnailPathFromPath(path));
}

std::filesystem::path EditorUtils::GetThumbnailPathFromPath(const std::filesystem::path& path)
{
	return path.string() + ".vtthumb.png";
}

std::string EditorUtils::GetDuplicatedNameFromEntity(const Volt::Entity& entity)
{
	std::string originalName = entity.GetTag();
	auto lastNumber = originalName.find_last_of("0123456789");
	auto lastUnderscore = originalName.find_last_of('_');

	int32_t currentNumber = 0;

	if (lastNumber != std::string::npos && lastUnderscore != std::string::npos && lastUnderscore < lastNumber)
	{
		std::string currentNumberStr = originalName.substr(lastUnderscore + 1, lastNumber - lastUnderscore);
		originalName = originalName.substr(0, lastUnderscore);
		currentNumber = std::stoi(currentNumberStr);
		currentNumber++;
	}

	originalName += "_" + std::to_string(currentNumber);
	return originalName;
}

void EditorUtils::MarkEntityAsEdited(const Volt::Entity& entity)
{
	auto scene = entity.GetScene();
	scene->MarkEntityAsEdited(entity);
}

void EditorUtils::MarkEntityAndChildrenAsEdited(const Volt::Entity& entity)
{
	auto scene = entity.GetScene();
	scene->MarkEntityAsEdited(entity);

	for (const auto& child : entity.GetChildren())
	{
		MarkEntityAndChildrenAsEdited(child);
	}
}
