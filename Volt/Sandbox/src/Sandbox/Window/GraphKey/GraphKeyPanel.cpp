#include "sbpch.h"
#include "GraphKeyPanel.h"

#include "Sandbox/Window/GraphKey/PinDrawing.h"
#include "Sandbox/Window/GraphKey/GraphKeyHelpers.h"

#include <Volt/Asset/AssetManager.h>
#include <Volt/Rendering/Texture/Texture2D.h>
#include <Volt/Utility/UIUtility.h>

#include <GraphKey/Graph.h>
#include <GraphKey/Node.h>
#include <GraphKey/Nodes/MathNodes.h>
#include <GraphKey/Nodes/PrintNodes.h>
#include <GraphKey/Nodes/BaseNodes.h>
#include <GraphKey/Nodes/UtilityNodes.h>

#include <builders.h>

namespace ed = ax::NodeEditor;
namespace utils = ax::NodeEditor::Utilities;

GraphKeyPanel::GraphKeyPanel()
	: EditorWindow("Graph Key", true)
{
	myInstance = this;

	CreateAttributeFunctions();
	CreateAttributeColors();

	ax::NodeEditor::Config cfg{};
	cfg.SettingsFile = "";

	myEditorContext = ed::CreateEditor(&cfg);
	myCurrentGraph = CreateRef<GraphKey::Graph>();
}

GraphKeyPanel::~GraphKeyPanel()
{
	ed::DestroyEditor(myEditorContext);
	myInstance = nullptr;
}

void GraphKeyPanel::UpdateMainContent()
{}

void GraphKeyPanel::UpdateContent()
{
	ed::SetCurrentEditor(myEditorContext);

	UpdateNodesPanel();
	UpdatePropertiesPanel();
	UpdateEditorPanel();

	ed::SetCurrentEditor(nullptr);
}

void GraphKeyPanel::SetActiveGraph(Ref<GraphKey::Graph> graph)
{
	myCurrentGraph = graph;
}

void GraphKeyPanel::UpdateEditorPanel()
{
	ImGui::Begin("Editor##graphKey", nullptr);
	ed::Begin("Editor##graphKey");

	if (myCurrentGraph)
	{
		for (const auto& node : myCurrentGraph->GetSpecification().nodes)
		{
			DrawNode(node);
		}
	}

	for (const auto& link : myCurrentGraph->GetSpecification().links)
	{
		ed::Link(ed::LinkId(link->id), ed::PinId(link->output), ed::PinId(link->input));
	}

	if (ed::BeginCreate())
	{
		ed::PinId startPinId = 0, endPinId = 0;
		if (ed::QueryNewLink(&startPinId, &endPinId))
		{
			auto* startAttr = myCurrentGraph->GetAttributeByID(startPinId.Get());
			auto* endAttr = myCurrentGraph->GetAttributeByID(endPinId.Get());

			if (startAttr && endAttr)
			{
				if (startAttr->direction == GraphKey::AttributeDirection::Input)
				{
					std::swap(startAttr, endAttr);
					std::swap(startPinId, endPinId);
				}

				bool sameType = false;
				if (startAttr->type == GraphKey::AttributeType::Flow && endAttr->type == GraphKey::AttributeType::Flow)
				{
					sameType = true;
				}

				if (!sameType)
				{
					if (startAttr->data.has_value() && endAttr->data.has_value())
					{
						sameType = startAttr->data.type() == endAttr->data.type();
					}
				}

				if (ed::AcceptNewItem() && startAttr->type == endAttr->type && sameType) // #TODO_Ivar: Check for correct type
				{
					myCurrentGraph->CreateLink(endAttr->id, startAttr->id);
				}
			}
		}
	}
	ed::EndCreate();

	ed::End();
	ImGui::End();
}

void GraphKeyPanel::CreateAttributeFunctions()
{
	myAttributeFunctions[std::type_index(typeid(bool))] = [](std::any& value) { GraphKeyHelpers::Attribute(std::any_cast<bool&>(value)); };
	myAttributeFunctions[std::type_index(typeid(int32_t))] = [](std::any& value) { GraphKeyHelpers::Attribute(std::any_cast<int32_t&>(value)); };
	myAttributeFunctions[std::type_index(typeid(uint32_t))] = [](std::any& value) { GraphKeyHelpers::Attribute(std::any_cast<uint32_t&>(value)); };
	myAttributeFunctions[std::type_index(typeid(int16_t))] = [](std::any& value) { GraphKeyHelpers::Attribute(std::any_cast<int16_t&>(value)); };
	myAttributeFunctions[std::type_index(typeid(uint16_t))] = [](std::any& value) { GraphKeyHelpers::Attribute(std::any_cast<uint16_t&>(value)); };
	myAttributeFunctions[std::type_index(typeid(int8_t))] = [](std::any& value) { GraphKeyHelpers::Attribute(std::any_cast<int8_t&>(value)); };
	myAttributeFunctions[std::type_index(typeid(uint8_t))] = [](std::any& value) { GraphKeyHelpers::Attribute(std::any_cast<uint8_t&>(value)); };
	myAttributeFunctions[std::type_index(typeid(double))] = [](std::any& value) { GraphKeyHelpers::Attribute(std::any_cast<double&>(value)); };
	myAttributeFunctions[std::type_index(typeid(float))] = [](std::any& value) { GraphKeyHelpers::Attribute(std::any_cast<float&>(value)); };
	myAttributeFunctions[std::type_index(typeid(gem::vec2))] = [](std::any& value) { GraphKeyHelpers::Attribute(std::any_cast<gem::vec2&>(value)); };
	myAttributeFunctions[std::type_index(typeid(gem::vec3))] = [](std::any& value) { GraphKeyHelpers::Attribute(std::any_cast<gem::vec3&>(value)); };
	myAttributeFunctions[std::type_index(typeid(gem::vec4))] = [](std::any& value) { GraphKeyHelpers::Attribute(std::any_cast<gem::vec4&>(value)); };
	myAttributeFunctions[std::type_index(typeid(gem::vec2ui))] = [](std::any& value) { GraphKeyHelpers::Attribute(std::any_cast<gem::vec2ui&>(value)); };
	myAttributeFunctions[std::type_index(typeid(gem::vec3ui))] = [](std::any& value) { GraphKeyHelpers::Attribute(std::any_cast<gem::vec3ui&>(value)); };
	myAttributeFunctions[std::type_index(typeid(gem::vec4ui))] = [](std::any& value) { GraphKeyHelpers::Attribute(std::any_cast<gem::vec4ui&>(value)); };
	myAttributeFunctions[std::type_index(typeid(std::string))] = [](std::any& value) { GraphKeyHelpers::Attribute(std::any_cast<std::string&>(value)); };
}

void GraphKeyPanel::CreateAttributeColors()
{
	///// Colors //////
	// Bool: Dark red
	// Int: Green
	// Float: Blue
	// String: Magenta
	// VectorN: Yellow
	// Default: Orange
	///////////////////

	myAttributeColors[std::type_index(typeid(bool))] = { 0.58f, 0.f, 0.01f, 1.f };
	myAttributeColors[std::type_index(typeid(int32_t))] = { 0.12f, 0.72f, 0.f, 1.f };
	myAttributeColors[std::type_index(typeid(uint32_t))] = { 0.12f, 0.72f, 0.f, 1.f };
	myAttributeColors[std::type_index(typeid(int16_t))] = { 0.12f, 0.72f, 0.f, 1.f };
	myAttributeColors[std::type_index(typeid(uint16_t))] = { 0.12f, 0.72f, 0.f, 1.f };
	myAttributeColors[std::type_index(typeid(int8_t))] = { 0.12f, 0.72f, 0.f, 1.f };
	myAttributeColors[std::type_index(typeid(uint8_t))] = { 0.12f, 0.72f, 0.f, 1.f };
	myAttributeColors[std::type_index(typeid(double))] = { 0.15f, 0.29f, 0.83f, 1.f };
	myAttributeColors[std::type_index(typeid(float))] = { 0.15f, 0.29f, 0.83f, 1.f };
	myAttributeColors[std::type_index(typeid(gem::vec2))] = { 0.96f, 0.99f, 0.f, 1.f };
	myAttributeColors[std::type_index(typeid(gem::vec3))] = { 0.96f, 0.99f, 0.f, 1.f };
	myAttributeColors[std::type_index(typeid(gem::vec4))] = { 0.96f, 0.99f, 0.f, 1.f };
	myAttributeColors[std::type_index(typeid(gem::vec2ui))] = { 0.96f, 0.99f, 0.f, 1.f };
	myAttributeColors[std::type_index(typeid(gem::vec3ui))] = { 0.96f, 0.99f, 0.f, 1.f };
	myAttributeColors[std::type_index(typeid(gem::vec4ui))] = { 0.96f, 0.99f, 0.f, 1.f };
	myAttributeColors[std::type_index(typeid(std::string))] = { 0.96f, 0.99f, 0.f, 1.f };

	myDefaultPinColor = { 0.99f, 0.51f, 0.f, 1.f };
}

void GraphKeyPanel::UpdateNodesPanel()
{
	ImGui::Begin("Nodes Panel");

	for (const auto& [name, func] : GraphKey::Registry::GetRegistry())
	{
		if (ImGui::Button(name.c_str()))
		{
			myCurrentGraph->AddNode(func());
		}
	}

	if (ImGui::Button("TestPlay")) //#TODO_Ivar: Change how this works
	{
		for (const auto& node : myCurrentGraph->GetSpecification().nodes)
		{
			if (node->GetName() == "Start")
			{
				Volt::OnScenePlayEvent e{};
				node->OnEvent(e);
				break;
			}
		}
	}

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
		const auto color = node->GetColor();

		builder.Header(ImColor{ color.x, color.y, color.z, color.w });
		{
			ImGui::Spring(0.f);
			ImGui::TextUnformatted(node->GetName().c_str());
		}
		ImGui::Spring(1.f);

		const float nodeHeaderHeight = 18.0f;
		ImGui::Dummy(ImVec2(0, nodeHeaderHeight));

		ImGui::Spring(0);
		builder.EndHeader();
	}

	GraphKeyHelpers::BeginAttributes();

	for (auto& input : node->inputs)
	{
		const auto typeIndex = std::type_index(input.data.type());
		builder.Input(ed::PinId(input.id));

		const bool connected = !input.links.empty();

		gem::vec4 color{};
		if (input.type == GraphKey::AttributeType::Flow)
		{
			color = { 1.f, 1.f, 1.f, 1.f };
		}
		else
		{
			if (myAttributeColors.contains(typeIndex))
			{
				color = myAttributeColors.at(typeIndex);
			}
			else
			{
				color = myDefaultPinColor;
			}
		}

		DrawPinIcon(input, connected, ImColor{ color.x, color.y, color.z, color.w }, 255);
		ImGui::Spring(0.f);
		ImGui::TextUnformatted(input.name.c_str());

		if (!input.hidden && !connected && input.data.has_value())
		{
			if (myAttributeFunctions.contains(typeIndex))
			{
				myAttributeFunctions.at(typeIndex)(input.data);
			}
		}

		ImGui::Spring(0.f);

		builder.EndInput();
	}

	for (auto& output : node->outputs)
	{
		const auto typeIndex = std::type_index(output.data.type());
		builder.Output(ed::PinId(output.id));

		const bool connected = !output.links.empty();

		ImGui::Spring(0.f);

		if (!output.hidden && !connected && output.data.has_value())
		{
			if (myAttributeFunctions.contains(typeIndex))
			{
				myAttributeFunctions.at(typeIndex)(output.data);
			}
		}

		ImGui::TextUnformatted(output.name.c_str());
		ImGui::Spring(0.f);

		gem::vec4 color{};

		if (output.type == GraphKey::AttributeType::Flow)
		{
			color = { 1.f, 1.f, 1.f, 1.f };
		}
		else
		{
			if (myAttributeColors.contains(typeIndex))
			{
				color = myAttributeColors.at(typeIndex);
			}
			else
			{
				color = myDefaultPinColor;
			}
		}

		DrawPinIcon(output, connected, ImColor{ color.x, color.y, color.z, color.w }, 255);
		builder.EndOutput();
	}

	GraphKeyHelpers::EndAttributes();

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
	for (const auto& node : myCurrentGraph->GetSpecification().nodes)
	{
		auto it = std::find_if(selectedNodeIds.begin(), selectedNodeIds.end(), [&](const ed::NodeId& id) 
			{
				return id.Get() == node->id;
			});

		if (it != selectedNodeIds.end())
		{
			selectedNodes.emplace_back(node);
		}
	}

	return selectedNodes;
}

const std::vector<Ref<GraphKey::Link>> GraphKeyPanel::GetSelectedLinks() const
{
	std::vector<ed::LinkId> selectedLinkIds;
	selectedLinkIds.resize(ed::GetSelectedObjectCount());

	int32_t linkCount = ed::GetSelectedLinks(selectedLinkIds.data(), (int32_t)selectedLinkIds.size());
	selectedLinkIds.resize(linkCount);

	std::vector<Ref<GraphKey::Link>> selectedLinks;
	for (const auto& link : myCurrentGraph->GetSpecification().links)
	{
		auto it = std::find_if(selectedLinkIds.begin(), selectedLinkIds.end(), [&](const ed::LinkId& id)
			{
				return id.Get() == link->id;
			});

		if (it != selectedLinkIds.end())
		{
			selectedLinks.emplace_back(link);
		}
	}

	return selectedLinks;
}