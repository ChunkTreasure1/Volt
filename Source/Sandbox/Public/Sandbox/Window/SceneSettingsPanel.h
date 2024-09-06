#pragma once

#include "Sandbox/Window/EditorWindow.h"

#include "Sandbox/Modals/Modal.h"

namespace Volt
{
	class Scene;
}

class SceneSettingsPanel : public EditorWindow
{
public:
	SceneSettingsPanel(Ref<Volt::Scene>& editorScene);

	void UpdateMainContent() override;

private:
	Ref<Volt::Scene>& m_editorScene;

	ModalID m_convertionModal = 0;
};
