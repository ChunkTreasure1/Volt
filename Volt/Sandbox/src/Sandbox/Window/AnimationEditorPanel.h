#pragma once

#include "Sandbox/Window/EditorWindow.h"

namespace Volt
{
	class Animation;
}

class AnimationEditorPanel : public EditorWindow
{
public:
	AnimationEditorPanel();
	~AnimationEditorPanel() override = default;

	void UpdateMainContent() override;
	void OpenAsset(Ref<Volt::Asset> asset) override;

	void OnOpen() override;
	void OnClose() override;

private:
	struct AddAnimEventData
	{
		uint32_t frame;
		std::string name;
	};

	void AddAnimationEventModal();

	Ref<Volt::Animation> m_animation;
	int32_t m_selectedKeyFrame = -1;
	AddAnimEventData m_addAnimEventData{};
};
