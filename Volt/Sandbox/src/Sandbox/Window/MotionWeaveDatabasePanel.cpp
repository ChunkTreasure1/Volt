#include "sbpch.h"
#include "MotionWeaveDatabasePanel.h"

#include "Sandbox/Utility/EditorUtilities.h"
#include <Volt/Utility/UIUtility.h>

#include <Volt/Asset/AssetManager.h>

#include <Volt/Asset/Animation/MotionWeaveDatabase.h>

MotionWeaveDatabasePanel::MotionWeaveDatabasePanel()
	: EditorWindow("Motion Weave Database")
{
}

void MotionWeaveDatabasePanel::UpdateMainContent()
{
		if (ImGui::Button("Save##MWDBSave"))
		{
			Volt::AssetManager::Get().SaveAsset(m_Database);
		}

	if (!m_Database)
	{
		return;
	}
	const auto& animHandles = m_Database->GetAnimationHandles();

	UI::BeginProperties();
	ImGui::BeginDisabled();
	for (int i = 0; i < animHandles.size(); i++)
	{
		const Volt::AssetHandle& animHandle = animHandles[i];
		Volt::AssetHandle tempHandle = animHandle;
		EditorUtils::Property("[" + std::to_string(i) + "]", tempHandle, Volt::AssetType::Animation);
	}
	ImGui::EndDisabled();


	Volt::AssetHandle tempHandle = Volt::Asset::Null();
	if (EditorUtils::Property("Add Animation", tempHandle, Volt::AssetType::Animation))
	{
		m_Database->AddAnimation(tempHandle);
	}
	UI::EndProperties();
}

void MotionWeaveDatabasePanel::UpdateContent()
{

}

void MotionWeaveDatabasePanel::OpenAsset(Ref<Volt::Asset> asset)
{
	m_Database = std::static_pointer_cast<Volt::MotionWeaveDatabase>(asset);
}

void MotionWeaveDatabasePanel::OnEvent(Volt::Event& e)
{
}

void MotionWeaveDatabasePanel::OnOpen()
{
}

void MotionWeaveDatabasePanel::OnClose()
{
}
