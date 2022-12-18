#pragma once

#include "Sandbox/Window/EditorWindow.h"

#include <imgui_node_editor.h>

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

private:
	void UpdateNodesPanel();
	void UpdatePropertiesPanel();
	void UpdateEditorPanel();

	void DrawNode(Ref<GraphKey::Node> node);

	const std::vector<Ref<GraphKey::Node>> GetSelectedNodes() const;
	const std::vector<Ref<GraphKey::Link>> GetSelectedLinks() const;

	ax::NodeEditor::EditorContext* myEditorContext = nullptr;
	Ref<GraphKey::Graph> myCurrentGraph;
};