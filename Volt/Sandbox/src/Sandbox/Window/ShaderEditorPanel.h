#pragma once

#include "Sandbox/Window/EditorWindow.h"

namespace Volt
{
	class Shader;
}

class ShaderEditorPanel : public EditorWindow
{
public:
	ShaderEditorPanel();

	void UpdateMainContent() override;
	void UpdateContent() override;

	void OpenAsset(Ref<Volt::Asset> asset) override;

	void OnClose() override;

private:
	void UpdateToolbar();
	void UpdateMainPanel();

	const float myButtonSize = 22.f;
	Ref<Volt::Shader> myCurrentShader;
};
