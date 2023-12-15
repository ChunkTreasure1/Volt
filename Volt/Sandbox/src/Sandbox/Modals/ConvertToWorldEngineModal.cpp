#include "sbpch.h"
#include "ConvertToWorldEngineModal.h"

#include <Volt/Asset/AssetManager.h>
#include <Volt/Scene/Scene.h>

#include <Volt/Utility/FileSystem.h>

#include <imgui.h>

ConvertToWorldEngineModal::ConvertToWorldEngineModal(const std::string& stringId)
	: Modal(stringId)
{
}

ConvertToWorldEngineModal::~ConvertToWorldEngineModal()
{
}

void ConvertToWorldEngineModal::DrawModalContent()
{
	ImGui::Text("Are you sure you want to convert to WorldEngine?");

	ImGui::SameLine();

	if (ImGui::Button("Convert"))
	{
		m_scene->GetSceneSettingsMutable().useWorldEngine = true;

		const auto& metadata = Volt::AssetManager::GetMetadataFromHandle(m_scene->handle);
		if (metadata.IsValid())
		{
			const std::filesystem::path directoryPath = Volt::AssetManager::GetFilesystemPath(metadata.filePath);
			const std::filesystem::path layersPath = directoryPath / "Layers";

			if (std::filesystem::exists(directoryPath) && std::filesystem::exists(layersPath))
			{
				FileSystem::MoveToRecycleBin(layersPath);
			}
		}

		Volt::AssetManager::SaveAsset(m_scene.GetSharedPtr());
		Close();
	}

	ImGui::SameLine();

	if (ImGui::Button("Cancel"))
	{
		m_scene->GetSceneSettingsMutable().useWorldEngine = false;
		Close();
	}
}

void ConvertToWorldEngineModal::OnClose()
{
	m_scene.Reset();
}
