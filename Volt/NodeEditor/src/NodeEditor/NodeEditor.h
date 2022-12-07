#pragma once

#include <imgui_node_editor.h>
#include <memory>

namespace NE
{
	struct EditorStyle
	{
		float linkStrength = 100.f;
	};

	struct EditorSettings
	{
		std::string name = "DefaultEditor";
		EditorStyle style;
	};

	class Graph;
	class Node;
	struct Link;

	class NodeEditor
	{
	public:
		NodeEditor(EditorSettings settings = {});
		~NodeEditor();

		void Draw();
		void Update();

		void SetActive();
		void SetStyle(const EditorStyle& style);

		const std::vector<std::shared_ptr<Node>> GetSelectedNodes() const;
		const std::vector<std::shared_ptr<Link>> GetSelectedLinks() const;

		inline void SetGraph(std::shared_ptr<Graph> graph) { myGraph = graph; }

	protected:
		ax::NodeEditor::EditorContext* myEditorContext = nullptr;
		EditorSettings mySettings;

		std::shared_ptr<Graph> myGraph;
	};
}