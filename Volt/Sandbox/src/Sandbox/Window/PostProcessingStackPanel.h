#pragma once

#include "Sandbox/Window/EditorWindow.h"

namespace Volt
{
	class PostProcessingStack;
}

class PostProcessingStackPanel : public EditorWindow
{
public:
	PostProcessingStackPanel();
	~PostProcessingStackPanel() override = default;

	void OpenAsset(Ref<Volt::Asset> asset) override;
	void UpdateMainContent() override;
	void OnClose() override;

private:
	Ref<Volt::PostProcessingStack> myCurrentStack;
};
