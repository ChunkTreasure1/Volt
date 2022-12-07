#pragma once

#include "Sandbox/Window/EditorWindow.h"

#include <NodeEditor/NodeEditor.h>

namespace Volt
{
	class RenderPipeline;
}

class TestNodeEditor : public EditorWindow
{
public:
	TestNodeEditor();

	void UpdateMainContent() override;
	void UpdateContent() override;

private:
	void UpdateNodesPanel();
	void UpdatePropertiesPanel();

	Ref<NE::Graph> myGraph;
	Ref<NE::NodeEditor> myEditor;
	Ref<Volt::RenderPipeline> myRenderPipline;
};