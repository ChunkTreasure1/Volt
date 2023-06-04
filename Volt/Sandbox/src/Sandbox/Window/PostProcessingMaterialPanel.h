#pragma once

#include "Sandbox/Window/EditorWindow.h"

namespace Volt
{
	class PostProcessingMaterial;
}

class PostProcessingMaterialPanel : public EditorWindow
{
public:
	PostProcessingMaterialPanel();
	~PostProcessingMaterialPanel() override = default;

	void OpenAsset(Ref<Volt::Asset> asset) override;
	void UpdateMainContent() override;
	void OnClose() override;

private:
	Ref<Volt::PostProcessingMaterial> myCurrentMaterial;
};
