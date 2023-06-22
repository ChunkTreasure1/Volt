#include "sbpch.h"
#include "BlendSpaceEditorPanel.h"

#include <Volt/Animation/BlendSpace.h>
#include <Volt/Asset/Animation/Animation.h>

#include <Volt/Utility/UIUtility.h>

#include "Sandbox/Utility/EditorUtilities.h"

BlendSpaceEditorPanel::BlendSpaceEditorPanel()
	: EditorWindow("Blend Space Editor", true)
{
}

BlendSpaceEditorPanel::~BlendSpaceEditorPanel()
{
}

void BlendSpaceEditorPanel::UpdateContent()
{
	UpdateProperties();
}

void BlendSpaceEditorPanel::OnEvent(Volt::Event& e)
{
}

void BlendSpaceEditorPanel::OpenAsset(Ref<Volt::Asset> asset)
{
	myCurrentBlendSpace = std::reinterpret_pointer_cast<Volt::BlendSpace>(asset);
}

void BlendSpaceEditorPanel::UpdateProperties()
{
	ImGui::SetNextWindowClass(GetWindowClass());
	ImGui::Begin("Properties##charEdit");
	ForceWindowDocked(ImGui::GetCurrentWindow());

	if (!myCurrentBlendSpace)
	{
		ImGui::End();
		return;
	}

	if (ImGui::Button("Save"))
	{
		Volt::AssetManager::Get().SaveAsset(myCurrentBlendSpace);
	}

	const std::vector<const char*> blendSpaceDimenstions =
	{
		"1D",
		"2D"
	};

	UI::Header("Properties");

	if (UI::BeginProperties("BlendSpaceProperties"))
	{
		int32_t currentDim = (int32_t)myCurrentBlendSpace->GetDimension();
		if (UI::ComboProperty("Dimension", currentDim, blendSpaceDimenstions))
		{
			myCurrentBlendSpace->SetDimension((Volt::BlendSpaceDimension)currentDim);
		}

		UI::EndProperties();
	}

	UI::Header("Animations");
	if (ImGui::Button("+"))
	{
		myAddHandle = 0;
	}
	if (ImGui::BeginPopupContextItem("addAnimPopup", ImGuiPopupFlags_MouseButtonLeft))
	{
		if (UI::BeginProperties("addAnimProperties"))
		{
			EditorUtils::Property("Animation", myAddHandle, Volt::AssetType::Animation);

			UI::EndProperties();
		}

		if (ImGui::Button("Add"))
		{
			myCurrentBlendSpace->AddAnimation(myAddHandle, { 0.f, 0.f });
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	auto& animations = const_cast<std::vector<std::pair<glm::vec2, Volt::AssetHandle>>&>(myCurrentBlendSpace->GetAnimations());

	if (UI::BeginProperties("animationValues"))
	{
		for (auto& [value, anim] : animations)
		{
			Ref<Volt::Animation> animAsset = Volt::AssetManager::GetAsset<Volt::Animation>(anim);

			std::string animName = "Null";

			if (animAsset && animAsset->IsValid())
			{
				animName = animAsset->name;
			}
			UI::Property(animName, value);
		}

		UI::EndProperties();
	}
	ImGui::End();
}
