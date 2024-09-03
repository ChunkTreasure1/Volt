#pragma once

#include "Sandbox/Window/EditorWindow.h"

#include <Volt/Events/ApplicationEvents.h>

namespace Volt
{
	class Event;
	class MotionWeaveDatabase;
}

class MotionWeaveDatabasePanel : public EditorWindow
{
public:
	MotionWeaveDatabasePanel();
	~MotionWeaveDatabasePanel() override = default;

	void UpdateMainContent() override;
	void UpdateContent() override;
	
	void OpenAsset(Ref<Volt::Asset> asset) override;

	void OnOpen() override;
	void OnClose() override;

private:
	Ref<Volt::MotionWeaveDatabase> m_Database;
};
