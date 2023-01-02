#include "nepch.h"
#include "NodeEditor.h"

#include "NodeEditor/Graph.h"

namespace NE
{
	namespace ed = ax::NodeEditor;

	NodeEditor::NodeEditor(EditorSettings settings)
		: mySettings(settings)
	{
		ax::NodeEditor::Config cfg{};
		cfg.SettingsFile = "";

		myEditorContext = ed::CreateEditor();

		SetStyle(settings.style);
	}

	NodeEditor::~NodeEditor()
	{
		ed::DestroyEditor(myEditorContext);
	}

	void NodeEditor::Draw()
	{
		ImGui::Begin(mySettings.name.c_str());
		ed::Begin(mySettings.name.c_str());

		if (myGraph)
		{
			for (const auto& node : myGraph->GetNodes())
			{
				node->Draw();
			}
		}

		for (const auto& link : myGraph->GetLinks())
		{
			auto val = ed::GetStyle().LinkStrength;
			ed::Link(link->id, link->startPin, link->endPin);
		}

		if (ed::BeginCreate())
		{
			ed::PinId startPinId = 0, endPinId = 0;
			if (ed::QueryNewLink(&startPinId, &endPinId))
			{
				auto startPin = myGraph->FindPin(startPinId);
				auto endPin = myGraph->FindPin(endPinId);

				if (startPin && endPin)
				{
					if (startPin->mode == PinMode::Input)
					{
						std::swap(startPin, endPin);
						std::swap(startPinId, endPinId);
					}

					if (ed::AcceptNewItem() && (startPin->type == endPin->type) && (startPin->node != endPin->node))
					{
						myGraph->AddLink(startPinId, endPinId);

						startPin->node->OnLinked(startPin, endPin->userData);
						endPin->node->OnLinked(endPin, startPin->userData);
					}
				}
			}
		}
		ed::EndCreate();

		ed::End();
		ImGui::End();
	}

	void NodeEditor::Update()
	{}

	void NodeEditor::SetActive()
	{
		ed::SetCurrentEditor(myEditorContext);
	}

	const std::vector<std::shared_ptr<Node>> NodeEditor::GetSelectedNodes() const
	{
		std::vector<ed::NodeId> selectedNodeIds;
		selectedNodeIds.resize(ed::GetSelectedObjectCount());

		int32_t nodeCount = ed::GetSelectedNodes(selectedNodeIds.data(), (int32_t)selectedNodeIds.size());
		selectedNodeIds.resize(nodeCount);

		std::vector<std::shared_ptr<Node>> selectedNodes;
		for (const auto& node : myGraph->GetNodes())
		{
			if (std::find(selectedNodeIds.begin(), selectedNodeIds.end(), node->GetId()) != selectedNodeIds.end())
			{
				selectedNodes.emplace_back(node);
			}
		}

		return selectedNodes;
	}

	const std::vector<std::shared_ptr<Link>> NodeEditor::GetSelectedLinks() const
	{
		std::vector<ed::LinkId> selectedLinkIds;
		selectedLinkIds.resize(ed::GetSelectedObjectCount());

		int32_t linkCount = ed::GetSelectedLinks(selectedLinkIds.data(), (int32_t)selectedLinkIds.size());
		selectedLinkIds.resize(linkCount);

		std::vector<std::shared_ptr<Link>> selectedLinks;
		for (const auto& link : myGraph->GetLinks())
		{
			if (std::find(selectedLinkIds.begin(), selectedLinkIds.end(), link->id) != selectedLinkIds.end())
			{
				selectedLinks.emplace_back(link);
			}
		}

		return selectedLinks;
	}

	void NodeEditor::SetStyle(const EditorStyle& style)
	{
		ed::GetStyle().LinkStrength = style.linkStrength;
	}
}