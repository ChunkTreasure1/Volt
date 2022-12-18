#include "sbpch.h"
#include "GraphKeyPanel.h"

#include <Volt/Asset/AssetManager.h>
#include <Volt/Rendering/Texture/Texture2D.h>
#include <Volt/Utility/UIUtility.h>

#include <GraphKey/Graph.h>
#include <GraphKey/Node.h>
#include <GraphKey/Nodes/MathNodes.h>

#include <builders.h>

namespace ed = ax::NodeEditor;
namespace utils = ax::NodeEditor::Utilities;

GraphKeyPanel::GraphKeyPanel()
	: EditorWindow("Graph Key", true)
{
	ax::NodeEditor::Config cfg{};
	cfg.SettingsFile = "";

	myEditorContext = ed::CreateEditor(&cfg);

	myCurrentGraph = CreateRef<GraphKey::Graph>();
}

GraphKeyPanel::~GraphKeyPanel()
{
	ed::DestroyEditor(myEditorContext);
}

void GraphKeyPanel::UpdateMainContent()
{}

void GraphKeyPanel::UpdateContent()
{
	UpdateNodesPanel();
	UpdatePropertiesPanel();
	UpdateEditorPanel();
}

void GraphKeyPanel::UpdateEditorPanel()
{
	ImGui::Begin("Editor##graphKey");
	ed::Begin("Editor##graphKey");

	if (myCurrentGraph)
	{
		for (const auto& node : myCurrentGraph->GetSpecification().nodes)
		{
			DrawNode(node);
		}
	}

	ed::End();
	ImGui::End();
}

void GraphKeyPanel::UpdateNodesPanel()
{
	ImGui::Begin("Nodes Panel");

	if (ImGui::Button("Add"))
	{
		myCurrentGraph->AddNode(CreateRef<GraphKey::AddNode>());
	}

	//if (ImGui::Button("Framebuffer"))
	//{
	//	myGraph->AddNode(CreateRef<FramebufferNode>());
	//}

	//if (ImGui::Button("Splitter"))
	//{
	//	myGraph->AddNode(CreateRef<FramebufferSplitNode>());
	//}


	ImGui::End();
}

void GraphKeyPanel::DrawNode(Ref<GraphKey::Node> node)
{
	ImTextureID textureId = nullptr;
	int32_t width = 0;
	int32_t height = 0;

	const Ref<Volt::Texture2D> headerTexture = Volt::AssetManager::GetAsset<Volt::Texture2D>("Editor/Textures/Graph/Translucency.dds");
	if (headerTexture && headerTexture->IsValid())
	{
		textureId = UI::GetTextureID(headerTexture);
		width = headerTexture->GetWidth();
		height = headerTexture->GetHeight();
	}

	utils::BlueprintNodeBuilder builder{ textureId, width, height };

	builder.Begin(ed::NodeId(node->id));

	{
		builder.Header();
		{
			ImGui::Spring(0.f);
			ImGui::TextUnformatted(node->name.c_str());
		}
		ImGui::Spring(1.f);

		const float nodeHeaderHeight = 18.0f;
		ImGui::Dummy(ImVec2(0, nodeHeaderHeight));

		ImGui::Spring(0);
		builder.EndHeader();
	}

	for (const auto& input : node->inputs)
	{
		builder.Input(ed::PinId(input.id));
		//DrawPinIcon(input, false, 255);
		ImGui::Spring(0.f);

		ImGui::TextUnformatted(input.name.c_str());
		ImGui::Spring(0.f);

		builder.EndInput();
	}

	for (const auto& output : node->outputs)
	{
		builder.Output(ed::PinId(output.id));
		ImGui::Spring(0.f);
		ImGui::TextUnformatted(output.name.c_str());
		ImGui::Spring(0.f);
		//DrawPinIcon(*output, false, 255);
		builder.EndOutput();
	}

	builder.End();
}

void GraphKeyPanel::UpdatePropertiesPanel()
{
	ImGui::Begin("Properties##Nodes");

	const auto selectedNodes = GetSelectedNodes();
	if (selectedNodes.size() == 1)
	{
		//selectedNodes.at(0)->DrawContent();
	}

	ImGui::End();
}

const std::vector<Ref<GraphKey::Node>> GraphKeyPanel::GetSelectedNodes() const
{
	std::vector<ed::NodeId> selectedNodeIds;
	selectedNodeIds.resize(ed::GetSelectedObjectCount());

	int32_t nodeCount = ed::GetSelectedNodes(selectedNodeIds.data(), (int32_t)selectedNodeIds.size());
	selectedNodeIds.resize(nodeCount);

	std::vector<Ref<GraphKey::Node>> selectedNodes;
	//for (const auto& node : myCurrentGraph->GetSpecification().nodes)
	//{
	//	if (std::find(selectedNodeIds.begin(), selectedNodeIds.end(), node->id) != selectedNodeIds.end())
	//	{
	//		selectedNodes.emplace_back(node);
	//	}
	//}

	return selectedNodes;
}

const std::vector<Ref<GraphKey::Link>> GraphKeyPanel::GetSelectedLinks() const
{
	std::vector<ed::LinkId> selectedLinkIds;
	selectedLinkIds.resize(ed::GetSelectedObjectCount());

	int32_t linkCount = ed::GetSelectedLinks(selectedLinkIds.data(), (int32_t)selectedLinkIds.size());
	selectedLinkIds.resize(linkCount);

	std::vector<Ref<GraphKey::Link>> selectedLinks;
	//for (const auto& link : myCurrentGraph->GetSpecification().links)
	//{
	//	if (std::find(selectedLinkIds.begin(), selectedLinkIds.end(), link->id) != selectedLinkIds.end())
	//	{
	//		selectedLinks.emplace_back(link);
	//	}
	//}

	return selectedLinks;
}