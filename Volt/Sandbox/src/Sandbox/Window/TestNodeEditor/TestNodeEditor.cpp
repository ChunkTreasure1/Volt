#include "sbpch.h"
#include "TestNodeEditor.h"

#include "Sandbox/Window/TestNodeEditor/Nodes/RenderPassNode.h"
#include "Sandbox/Window/TestNodeEditor/Nodes/FramebufferNode.h"
#include "Sandbox/Window/TestNodeEditor/Nodes/FramebufferSplitNode.h"

#include <Volt/Rendering/RenderPipeline.h>
#include <NodeEditor/Graph.h>

TestNodeEditor::TestNodeEditor()
	: EditorWindow("Test Node Editor", true)
{
	myGraph = CreateRef<NE::Graph>();
	myEditor = CreateRef<NE::NodeEditor>();
	myEditor->SetGraph(myGraph);

	myRenderPipline = CreateRef<Volt::RenderPipeline>();
}

void TestNodeEditor::UpdateMainContent()
{}

void TestNodeEditor::UpdateContent()
{
	myEditor->Update();
	myEditor->Draw();

	UpdateNodesPanel();
	UpdatePropertiesPanel();
}

void TestNodeEditor::UpdateNodesPanel()
{
	ImGui::Begin("Nodes Panel");

	if (ImGui::Button("Render Pass"))
	{
	  	uint32_t index = myRenderPipline->AddRenderPass();
		myGraph->AddNode(CreateRef<RenderPassNode>(myRenderPipline, index));
	}

	if (ImGui::Button("Framebuffer"))
	{
		myGraph->AddNode(CreateRef<FramebufferNode>());
	}

	if (ImGui::Button("Splitter"))
	{
		myGraph->AddNode(CreateRef<FramebufferSplitNode>());
	}


	ImGui::End();
}

void TestNodeEditor::UpdatePropertiesPanel()
{
	ImGui::Begin("Properties##Nodes");

	const auto selectedNodes = myEditor->GetSelectedNodes();
	if (selectedNodes.size() == 1)
	{
		selectedNodes.at(0)->DrawContent();
	}

	ImGui::End();
}
