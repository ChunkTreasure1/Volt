#pragma once

#include "Sandbox/EditorCommandStack.h"

#include <Volt/Events/Event.h>
#include <imgui.h>
#include <imgui_internal.h>

#include <string>

namespace Volt
{
	class Asset;
}

class EditorWindow
{
public:
	EditorWindow(const std::string& title, bool dockSpace = false, std::string id = "");
	virtual ~EditorWindow() = default;

	bool Begin();
	void End();
	void Open();
	void Close();

	void SetMinWindowSize(ImVec2 minSize);

	void Focus();
	bool IsDocked() const;

	virtual void UpdateMainContent() = 0;
	virtual void UpdateContent() {}
	virtual void OnEvent(Volt::Event& e) {}
	virtual void OpenAsset(Ref<Volt::Asset> asset) {}

	inline const std::string& GetTitle() const { return myTitle; }
	inline const bool& IsOpen() const { return myIsOpen; }
	inline const bool IsFocused() const { return myIsFocused; }
	inline const bool IsHovered() const { return myIsHovered; }
	virtual void OnClose() {}
	virtual void OnOpen() {}

	inline EditorCommandStack& GetCommandStack() { return myCommandStack; }

protected:
	void ForceWindowDocked(ImGuiWindow* childWindow);
	inline ImGuiWindowClass* GetWindowClass() { return &myWindowClass; };

	std::string myTitle;
	std::string myId;
	EditorCommandStack myCommandStack{};

	ImGuiWindowFlags myWindowFlags = 0;
	ImGuiWindowFlags myNodePanelFlags = 0;

	ImVec2 myMinSize = { -1.f, -1.f };

	bool myIsOpen = false;
	bool myIsFocused = false;
	bool myIsHovered = false;
	bool myIsDocked = false;
	bool myHasDockSpace = false;

	bool myPreviousFrameOpen = false;

	ImGuiID myMainDockId = 0;
	ImGuiWindowClass myWindowClass;

private:
	std::unordered_map<ImGuiID, ImGuiID> myDockIds;
};
