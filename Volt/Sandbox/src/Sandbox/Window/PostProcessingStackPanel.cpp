#include "sbpch.h"
#include "PostProcessingStackPanel.h"

#include "Sandbox/Utility/EditorUtilities.h"

#include <Volt/Asset/AssetManager.h>
#include <Volt/Asset/Rendering/PostProcessingStack.h>

#include <Volt/Utility/UIUtility.h>

PostProcessingStackPanel::PostProcessingStackPanel()
	: EditorWindow("Post Processing Stack Editor")
{
}

void PostProcessingStackPanel::OpenAsset(Ref<Volt::Asset> asset)
{
	myCurrentStack = std::reinterpret_pointer_cast<Volt::PostProcessingStack>(asset);
}

void PostProcessingStackPanel::UpdateMainContent()
{
	if (ImGui::Button("Save") && myCurrentStack)
	{
		Volt::AssetManager::Get().SaveAsset(myCurrentStack);
	}

	ImGui::SameLine();

	if (ImGui::Button("Add") && myCurrentStack)
	{
		myCurrentStack->PushEffect({});
	}

	if (!myCurrentStack)
	{
		return;
	}

	if (UI::BeginProperties("effects"))
	{
		for (auto& effect : myCurrentStack->GetAllEffectsMutable())
		{
			EditorUtils::Property("Material", effect.materialHandle, Volt::AssetType::PostProcessingMaterial);
		}
		UI::EndProperties();
	}
}

void PostProcessingStackPanel::OnClose()
{
	myCurrentStack = nullptr;
}
