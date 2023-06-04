#pragma once

#include "Sandbox/Window/EditorWindow.h"
#include "Sandbox/NodeGraph/IONodeGraphEditor.h"
#include "Sandbox/NodeGraph/NodeGraphEditorBackend.h"

struct ScriptingBackend : public NodeGraph::EditorBackend
{
	~ScriptingBackend() override = default;
};

class GraphKeyPanel : public IONodeGraphEditor<GraphKey::GraphType::Scripting, ScriptingBackend>
{
public:
	GraphKeyPanel(Ref<Volt::Scene>& currentScene);
	~GraphKeyPanel() override;

	void OpenAsset(Ref<Volt::Asset> asset) override;

	bool SaveSettings(const std::string& data)  override;
	size_t LoadSettings(std::string& data)  override;

	bool SaveNodeSettings(const Volt::UUID nodeId, const std::string& data)  override;
	size_t LoadNodeSettings(const Volt::UUID nodeId, std::string& data)  override;

	void SetActiveGraph(Ref<GraphKey::Graph> graph);
	inline static GraphKeyPanel& Get() { return *myInstance; }

private:
	inline static GraphKeyPanel* myInstance = nullptr;
};
