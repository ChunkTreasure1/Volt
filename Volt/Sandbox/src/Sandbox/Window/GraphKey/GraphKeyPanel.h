#pragma once

#include "Sandbox/Window/EditorWindow.h"

#include <imgui_node_editor.h>
#include <typeindex>

namespace GraphKey
{
	class Graph;
	struct Node;
	struct Link;
}

class GraphKeyPanel : public EditorWindow
{
public:
	GraphKeyPanel();
	~GraphKeyPanel() override;

	void UpdateMainContent() override;
	void UpdateContent() override;

	void SetActiveGraph(Ref<GraphKey::Graph> graph);

	inline static GraphKeyPanel& Get() { return *myInstance; }

private:
	void UpdateNodesPanel();
	void UpdatePropertiesPanel();
	void UpdateEditorPanel();
	
	void CreateAttributeFunctions();
	void CreateAttributeColors();

	void DrawNode(Ref<GraphKey::Node> node);

	const std::vector<Ref<GraphKey::Node>> GetSelectedNodes() const;
	const std::vector<Ref<GraphKey::Link>> GetSelectedLinks() const;

	inline static GraphKeyPanel* myInstance = nullptr;

	ax::NodeEditor::EditorContext* myEditorContext = nullptr;
	Ref<GraphKey::Graph> myCurrentGraph;

	std::unordered_map<std::type_index, std::function<void(std::any& data)>> myAttributeFunctions;
	std::unordered_map<std::type_index, gem::vec4> myAttributeColors;

	gem::vec4 myDefaultPinColor;
};