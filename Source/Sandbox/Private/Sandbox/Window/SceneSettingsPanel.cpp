#include "sbpch.h"
#include "Window/SceneSettingsPanel.h"

#include "Sandbox/Modals/ConvertToWorldEngineModal.h"
#include "Sandbox/UISystems/ModalSystem.h"

#include <Volt/Utility/UIUtility.h>

SceneSettingsPanel::SceneSettingsPanel(Ref<Volt::Scene>& editorScene)
	: EditorWindow("Scene Settings"), m_editorScene(editorScene)
{
	auto& modal = ModalSystem::AddModal<ConvertToWorldEngineModal>("Convert To World Engine##sceneSettings");
	m_convertionModal = modal.GetID();
}

void SceneSettingsPanel::UpdateMainContent()
{
	auto& sceneSettings = m_editorScene->GetSceneSettingsMutable();

	if (UI::BeginProperties("sceneSettings"))
	{
		if (UI::Property("Use World Engine", sceneSettings.useWorldEngine) && sceneSettings.useWorldEngine)
		{
			ModalSystem::GetModal<ConvertToWorldEngineModal>(m_convertionModal).SetCurrentScene(m_editorScene);
			ModalSystem::GetModal<ConvertToWorldEngineModal>(m_convertionModal).Open();
		}

		UI::EndProperties();
	}
}
