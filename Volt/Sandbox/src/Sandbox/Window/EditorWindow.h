#pragma once

#include "Sandbox/EditorCommandStack.h"

#include <Volt/Events/Event.h>
#include <imgui.h>

#include <string>

namespace Volt
{
	class Asset;
}

class EditorWindow
{
public:
	EditorWindow(const std::string& title, bool dockSpace = false);
	virtual ~EditorWindow() = default;

	bool Begin();
	void End();
	void Open();
	void Close();

	void SetMinWindowSize(ImVec2 minSize);

	virtual void UpdateMainContent() = 0;
	virtual void UpdateContent() {}
	virtual void OnEvent(Volt::Event& e) {}
	virtual void OpenAsset(Ref<Volt::Asset> asset) {}

	inline const std::string& GetTitle() const { return myTitle; }
	inline const bool& IsOpen() const { return myIsOpen; }
	inline const bool IsFocused() const { return myIsFocused; }
	inline const bool IsHovered() const { return myIsHovered; }

	inline EditorCommandStack& GetCommandStack() { return myCommandStack; }

protected:
	std::string myTitle;
	EditorCommandStack myCommandStack{};

	ImGuiWindowFlags myWindowFlags = 0;
	ImGuiWindowFlags myNodePanelFlags = 0;

	ImVec2 myMinSize = { -1.f, -1.f };

	bool myIsOpen = false;
	bool myIsFocused = false;
	bool myIsHovered = false;
	bool myHasDockSpace = false;
};