#pragma once

#include "Sandbox/Window/EditorWindow.h"

#include <imgui_node_editor.h>
#include <typeindex>

namespace GraphKey
{
	class Graph;
	struct Node;
	struct Link;
	struct Attribute;
}

class GraphKeyPanel : public EditorWindow
{
public:
	GraphKeyPanel(Ref<Volt::Scene>& aScene);
	~GraphKeyPanel() override;

	void UpdateMainContent() override;
	void UpdateContent() override;

	void SetActiveGraph(Ref<GraphKey::Graph> graph);
	bool SetNodeSettings(const char* data);
	const std::string GetNodeSettings() const;

	inline static GraphKeyPanel& Get() { return *myInstance; }

private:
	void UpdateNodesPanel();
	void UpdatePropertiesPanel();
	void UpdateEditorPanel();
	void UpdateContextPopups();

	void InitializeEditor();

	void CreateAttributeFunctions();
	void CreateAttributeColors();

	void DrawNode(Ref<GraphKey::Node> node);
	Ref<GraphKey::Node> DrawNodeList(std::string& query);

	const gem::vec4 GetColorFromAttribute(const GraphKey::Attribute& attr);
	const bool CanLinkAttributes(ax::NodeEditor::PinId& input, ax::NodeEditor::PinId& output);

	const std::vector<Ref<GraphKey::Node>> GetSelectedNodes() const;
	const std::vector<Ref<GraphKey::Link>> GetSelectedLinks() const;

	inline static GraphKeyPanel* myInstance = nullptr;

	Ref<GraphKey::Graph> myCurrentGraph;
	Ref<Volt::Scene>& myCurrentScene;

	std::unordered_map<std::type_index, std::function<void(std::any& data)>> myAttributeFunctions;
	std::unordered_map<std::type_index, gem::vec4> myAttributeColors;
	
	gem::vec4 myDefaultPinColor;
	std::string mySearchQuery;
	std::string myContextSearchQuery;

	bool myCreateNewNode = false;
	Volt::UUID myNewNodeLinkPinId = Volt::UUID(0);
	Volt::UUID myNewLinkPinId = Volt::UUID(0);

	ax::NodeEditor::EditorContext* myEditorContext = nullptr;
	ax::NodeEditor::NodeId myContextNodeId;
	ax::NodeEditor::PinId myContextPinId;
	ax::NodeEditor::LinkId myContextLinkId;
};